#include "defines.h"
#include "kozos.h"
#include "intr.h"
#include "interrupt.h"
#include "syscall.h"
#include "lib.h"

#define THREAD_NUM	6
#define THREAD_NAME_SIZE	15

/* スレッド・コンテキスト */
typedef struct {
	uint32 sp;	/* スタック・ポインタ */
} kz_context;

/* タスク・コントロール・ブロック(TCB) */
typedef struct kz_thread {
	struct kz_thread *next;
	char name[THREAD_NAME_SIZE + 1];
	char *stack;

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
} readyque;

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
	readyque.head = current->next;
	if (readyque.head == NULL) {
		readyque.tail = NULL;
	}
	current->next = NULL;
	return 0;
}

/* カレント・スレッドをレディー・キューに繋げる */
static int putcurrent(void)
{
	if (current == NULL) {
		return -1;
	}
	if (readyque.tail) {
		readyque.tail->next = current;
	} else {
		readyque.head = current;
	}
	readyque.tail = current;
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
static kz_thread_id_t thread_run(kz_func_t func, char *name, int stacksize,
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

	/* プログラムカウンタを設定する */
	/* ディスパッチ時にプログラムカウンタに格納される値としてthread_initを設定する。
	 * よってスレッドはthread_initから動作を開始する。 */
	*(--sp) = (uint32) thread_init;

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
								   p->un.run.stacksize,
								   p->un.run.argc,
								   p->un.run.argv);
		break;
	case KZ_SYSCALL_TYPE_EXIT: /* kz_exit() */
		/* TCBが消去されるので、戻り値を書き込んではいけない */
		thread_exit();
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
	if (!readyque.head) {
		kz_sysdown();
	}
	current = readyque.head;
}

static void syscall_intr(void)
{
	syscall_proc(current->syscall.type, current->syscall.param);
}

static void softerr_intr(void)
{
	puts(current->name);
	puts("  DOWN.\n");
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
