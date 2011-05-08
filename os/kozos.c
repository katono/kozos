#include "defines.h"
#include "kozos.h"
#include "../intr.h"
#include "interrupt.h"
#include "syscall.h"
#include "lib.h"

#define THREAD_NUM	6
#define PRIORITY_NUM	16
#define THREAD_NAME_SIZE	15

/* スレッド・コンテキスト */
typedef struct {
	uint32 sp;	/* スタック・ポインタ */
} kz_context;

/* タスク・コントロール・ブロック(TCB) */
typedef struct kz_thread {
	struct kz_thread *next;
	char name[THREAD_NAME_SIZE + 1];
	int priority;
	char *stack;
	uint32 flags;
#define KZ_THREAD_FLAG_READY	(1 << 0)

	struct {	/* スレッドのスタートアップ(thread_init())に渡すパラメータ */
		kz_func_t func;
		int argc;
		char **argv;
	} init;

	struct {	/* システムコール用バッファ */
		kz_syscall_type_t type;
		kz_syscall_param_t *param;
	} syscall;

	kz_context context;
} kz_thread;

/* スレッドのレディー・キュー */
static struct {
	kz_thread *head;
	kz_thread *tail;
} readyque[PRIORITY_NUM];

static kz_thread *current;
static kz_thread threads[THREAD_NUM];
static kz_handler_t handlers[SOFTVEC_TYPE_NUM];

void dispatch(kz_context *context);


/* カレント・スレッドをレディー・キューから抜き出す */
static int getcurrent(void)
{
	if (current == NULL) {
		return -1;
	}
	if (!(current->flags & KZ_THREAD_FLAG_READY)) {
		/* すでに無い場合は無視 */
		return 1;
	}
	/* カレント・スレッドは必ず先頭にあるはずなので、先頭から抜き出す */
	readyque[current->priority].head = current->next;
	if (readyque[current->priority].head == NULL) {
		readyque[current->priority].tail = NULL;
	}
	current->flags &= ~KZ_THREAD_FLAG_READY;
	current->next = NULL;
	return 0;
}

/* カレント・スレッドをレディー・キューに繋げる */
static int putcurrent(void)
{
	if (current == NULL) {
		return -1;
	}
	if (current->flags & KZ_THREAD_FLAG_READY) {
		/* すでに有る場合は無視 */
		return 1;
	}
	if (readyque[current->priority].tail) {
		readyque[current->priority].tail->next = current;
	} else {
		readyque[current->priority].head = current;
	}
	readyque[current->priority].tail = current;
	current->flags |= KZ_THREAD_FLAG_READY;
	return 0;
}


/* スレッドの終了 */
static void thread_end(void)
{
	kz_exit();
}

/* スレッドのスタートアップ */
static void thread_init(kz_thread *thp)
{
	/* スレッドのメイン関数を呼び出す */
	thp->init.func(thp->init.argc, thp->init.argv);
	thread_end();
}

/* システムコールの処理(kz_run():スレッドの起動) */
static kz_thread_id_t thread_run(kz_func_t func, char *name, int priority, int stacksize,
									int argc, char *argv[])
{
	int i;
	kz_thread *thp;
	uint32 *sp;
	extern char userstack; /* リンカスクリプトで定義されるスタック領域 */
	static char *thread_stack = &userstack;

	/* 空いているタスク・コントロール・ブロックを検索 */
	for (i = 0; i < THREAD_NUM; i++) {
		thp = &threads[i];
		if (!thp->init.func) {
			/* 見つかった */
			break;
		}
	}
	if (i == THREAD_NUM) {
		/* 見つからなかった */
		return -1;
	}

	memset(thp, 0, sizeof *thp);

	/* タスク・コントロール・ブロック(TCB)の設定 */
	strcpy(thp->name, name);
	thp->next = NULL;
	thp->priority = priority;
	thp->flags = 0;
	thp->init.func = func;
	thp->init.argc = argc;
	thp->init.argv = argv;

	/* スタック領域を獲得 */
	memset(thread_stack, 0, stacksize);
	thread_stack += stacksize;

	thp->stack = thread_stack;

	/* スタックの初期化 */
	sp = (uint32 *) thp->stack;
	/* スタックにthread_initからの戻り先としてthread_endを設定する */
	*(--sp) = (uint32) thread_end;

	/* 
	 * プログラムカウンタを設定する
	 * スレッドの優先度がゼロの場合には、割り込み禁止スレッドとする。
	 */
	/* ディスパッチ時にプログラムカウンタに格納される値としてthread_initを設定する。
	 * よってスレッドはthread_initから動作を開始する。 */
	*(--sp) = (uint32) thread_init | ((uint32)(priority ? 0 : 0xc0) << 24);;

	*(--sp) = 0; /* ER6 */
	*(--sp) = 0; /* ER5 */
	*(--sp) = 0; /* ER4 */
	*(--sp) = 0; /* ER3 */
	*(--sp) = 0; /* ER2 */
	*(--sp) = 0; /* ER1 */

	/* スレッドのスタートアップ(thread_init)に渡す引数 */
	*(--sp) = (uint32) thp; /* ER0 */

	/* スレッドのコンテキストを設定 */
	thp->context.sp = (uint32) sp;

	/* システムコールを呼び出したスレッドをレディー・キューに戻す */
	putcurrent();

	/* 新規作成したスレッドを、レディー・キューに接続する */
	current = thp;
	putcurrent();

	return (kz_thread_id_t) current;
}

/* システムコールの処理(kz_exit():スレッドの終了) */
static int thread_exit(void)
{
	/* 
	 * 本来ならスタックも解放して再利用できるようにすべきだが省略。
	 * このため、スレッドを頻繁に生成・消去するようなことは現状でできない。
	 */
	puts(current->name);
	puts(" EXIT.\n");
	memset(current, 0, sizeof *current);
	return 0;
}

/* システムコールの処理(kz_wait():スレッドの実行権放棄) */
static int thread_wait(void)
{
	putcurrent();
	return 0;
}

/* システムコールの処理(kz_sleep():スレッドのスリープ) */
static int thread_sleep(void)
{
	/* レディー・キューから外されたままになるので、スケジューリングされなくなる */
	return 0;
}

/* システムコールの処理(kz_wakeup():スレッドのウェイクアップ) */
static int thread_wakeup(kz_thread_id_t id)
{
	/* ウェイクアップを呼び出したスレッドをレディー・キューに戻す */
	putcurrent();

	/* 指定されたスレッドをレディー・キューに接続してウェイクアップする */
	current = (kz_thread *) id;
	putcurrent();

	return 0;
}

/* システムコールの処理(kz_getid():スレッドID取得) */
static kz_thread_id_t thread_getid(void)
{
	putcurrent();
	return (kz_thread_id_t) current;
}

/* システムコールの処理(kz_chpri():スレッドの優先度変更) */
static int thread_chpri(int priority)
{
	int old = current->priority;
	if (priority >= 0) {
		current->priority = priority;
	}
	putcurrent();
	return old;
}


/* 割り込みハンドらの登録 */
static int setintr(softvec_type_t type, kz_handler_t handler)
{
	static void thread_intr(softvec_type_t type, unsigned long sp);

	/* 
	 * 割り込みを受け付けるために、ソフトウェア割り込みベクタに
	 * OSの割り込み処理の入口となる関数を登録する。
	 */
	softvec_setintr(type, thread_intr);

	handlers[type] = handler; /* OS側から呼び出す割り込みハンドラを登録 */

	return 0;
}

static void call_functions(kz_syscall_type_t type, kz_syscall_param_t *p)
{
	/* システムコールの実行中にcurrentが書き換わるので注意 */
	switch (type) {
	case KZ_SYSCALL_TYPE_RUN: /* kz_run() */
		p->un.run.ret = thread_run(p->un.run.func,
								   p->un.run.name,
								   p->un.run.priority,
								   p->un.run.stacksize,
								   p->un.run.argc,
								   p->un.run.argv);
		break;
	case KZ_SYSCALL_TYPE_EXIT: /* kz_exit() */
		/* TCBが消去されるので、戻り値を書き込んではいけない */
		thread_exit();
		break;
	case KZ_SYSCALL_TYPE_WAIT: /* kz_wait() */
		p->un.wait.ret = thread_wait();
		break;
	case KZ_SYSCALL_TYPE_SLEEP: /* kz_sleep() */
		p->un.sleep.ret = thread_sleep();
		break;
	case KZ_SYSCALL_TYPE_WAKEUP: /* kz_wakeup() */
		p->un.wakeup.ret = thread_wakeup(p->un.wakeup.id);
		break;
	case KZ_SYSCALL_TYPE_GETID: /* kz_getid() */
		p->un.getid.ret = thread_getid();
		break;
	case KZ_SYSCALL_TYPE_CHPRI: /* kz_chpri() */
		p->un.chpri.ret = thread_chpri(p->un.chpri.priority);
		break;
	default:
		break;
	}
}

/* システムコールの処理 */
static void syscall_proc(kz_syscall_type_t type, kz_syscall_param_t *p)
{
	/* 
	 * システムコールを呼び出したスレッドをレディーキューから
	 * 外した状態で処理関数を呼び出す。このためシステムコールを
	 * 呼び出したスレッドをそのまま動作継続させたい場合には、
	 * 処理関数の内部でputcurrent()を行う必要がある。
	 */
	getcurrent();
	call_functions(type, p);
}

/* スレッドのスケジューリング */
static void schedule(void)
{
	int i;

	/* 
	 * 優先順位の高い順(優先度の数値の小さい順)にレディー・キューをみて
	 * 動作可能なスレッドを検索する
	 */
	for (i = 0; i < PRIORITY_NUM; i++) {
		if (readyque[i].head) {
			/* 見つかった */
			break;
		}
	}
	if (i == PRIORITY_NUM) {
		/* 見つからなかった */
		kz_sysdown();
	}
	current = readyque[i].head;
}

static void syscall_intr(void)
{
	syscall_proc(current->syscall.type, current->syscall.param);
}

static void softerr_intr(void)
{
	puts(current->name);
	puts(" DOWN.\n");
	getcurrent(); /* レディーキューから外す */
	thread_exit(); /* スレッド終了する */
}

/* 割り込み処理の入口関数 */
static void thread_intr(softvec_type_t type, unsigned long sp)
{
	/* カレントスレッドのコンテキストを保存する */
	current->context.sp = sp;

	/* 
	 * 割り込みごとの処理を実行する。
	 * SOFTVEC_TYPE_SYSCALL, SOFTVEC_TYPE_SOFTERRの場合は
	 * syscall_intr(), softerr_intr()がハンドラに登録されているので、
	 * それらが実行される。
	 */
	if (handlers[type]) {
		handlers[type]();
	}

	schedule(); /* スレッドのスケジューリング */

	/* スレッドのディスパッチ
	 * dispatch()はstartup.sにある。 */
	dispatch(&current->context);
	/* ここには返ってこない */
}

void kz_start(kz_func_t func, char *name, int priority, int stacksize, int argc, char *argv[])
{
	/* 
	 * 以降で呼び出すスレッド関連のライブラリ関数の内部でcurrentを
	 * 見ている場合があるので、currentをNULLに初期化しておく。
	 */
	current = NULL;

	memset(readyque, 0, sizeof readyque);
	memset(threads, 0, sizeof threads);
	memset(handlers, 0, sizeof handlers);

	/* 割り込みハンドラの登録 */
	setintr(SOFTVEC_TYPE_SYSCALL, syscall_intr); /* システムコール */
	setintr(SOFTVEC_TYPE_SOFTERR, softerr_intr); /* ダウン要因発生 */

	/* システムコール発行不可なので直接関数を呼び出してスレッドを作成する */
	current = (kz_thread *) thread_run(func, name, priority, stacksize, argc, argv);

	/* 最初のスレッドを起動 */
	dispatch(&current->context);

	/* ここには返ってこない */
}

void kz_sysdown(void)
{
	puts("system error!\n");
	while (1);
}

/* システムコール呼び出しようライブラリ関数 */
void kz_syscall(kz_syscall_type_t type, kz_syscall_param_t *param)
{
	current->syscall.type = type;
	current->syscall.param = param;
	asm volatile ("trapa #0"); /* トラップ割り込み発行 */
}

