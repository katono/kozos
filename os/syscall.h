#ifndef KOZOS_SYSCALL_H_INCLUDED
#define KOZOS_SYSCALL_H_INCLUDED

#include "defines.h"

/* システムコール番号の定義 */
typedef enum {
	KZ_SYSCALL_TYPE_RUN = 0,
	KZ_SYSCALL_TYPE_EXIT,
	KZ_SYSCALL_TYPE_WAIT,
	KZ_SYSCALL_TYPE_SLEEP,
	KZ_SYSCALL_TYPE_WAKEUP,
	KZ_SYSCALL_TYPE_GETID,
	KZ_SYSCALL_TYPE_CHPRI,
	KZ_SYSCALL_TYPE_KMALLOC,
	KZ_SYSCALL_TYPE_KMFREE,
	KZ_SYSCALL_TYPE_SEND,
	KZ_SYSCALL_TYPE_RECV
} kz_syscall_type_t;

/* システムコール呼び出し時のパラメータ格納域の定義 */
typedef struct {
	union {
		struct {
			kz_func_t func;
			char *name;
			int priority;
			int stacksize;
			int argc;
			char **argv;
			kz_thread_id_t ret;
		} run;
		struct {
			int dummy;
		} exit;
		struct {
			int ret;
		} wait;
		struct {
			int ret;
		} sleep;
		struct {
			kz_thread_id_t id;
			int ret;
		} wakeup;
		struct {
			kz_thread_id_t ret;
		} getid;
		struct {
			int priority;
			int ret;
		} chpri;
		struct {
			int size;
			void *ret;
		} kmalloc;
		struct {
			void *p;
			int ret;
		} kmfree;
		struct {
			kz_msgbox_id_t id;
			int size;
			char *p;
			int ret;
		} send;
		struct {
			kz_msgbox_id_t id;
			int *sizep;
			char **pp;
			kz_thread_id_t ret;
		} recv;
	} un;
} kz_syscall_param_t;

#endif /* KOZOS_SYSCALL_H_INCLUDED */
