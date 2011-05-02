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

