#ifndef DEFINES_H_INCLUDED
#define DEFINES_H_INCLUDED

#include "stddef.h"

#define STATIC_ASSERT(expr) \
    extern void static_assert_func_(int static_assert_[(expr)?+1:-1])

#define SERIAL_DEFAULT_DEVICE 1

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned long uint32;

typedef uint32 kz_thread_id_t;
typedef int (*kz_func_t)(int argc, char *argv[]);
typedef void (*kz_handler_t)(void);

#endif /* DEFINES_H_INCLUDED */
