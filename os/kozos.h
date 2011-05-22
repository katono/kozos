#ifndef KOZOS_H_INCLUDED
#define KOZOS_H_INCLUDED

#include "defines.h"
#include "syscall.h"

/* システムコール */
kz_thread_id_t kz_run(kz_func_t func, char *name, int priority, int stacksize,
						int argc, char *argv[]);
void kz_exit(void);
int kz_wait(void);
int kz_sleep(void);
int kz_wakeup(kz_thread_id_t id);
kz_thread_id_t kz_getid(void);
int kz_chpri(int priority);
void *kz_kmalloc(int size);
int kz_kmfree(void *p);

/* ライブラリ関数 */
void kz_start(kz_func_t func, char *name, int priority, int stacksize,
						int argc, char *argv[]);
void kz_sysdown(void);
void kz_syscall(kz_syscall_type_t type, kz_syscall_param_t *param);

/* ユーザースレッド */
int test10_1_main(int argc, char *argv[]);

#endif /* KOZOS_H_INCLUDED */
