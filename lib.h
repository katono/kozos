#ifndef LIB_H_INCLUDED
#define LIB_H_INCLUDED

#include <stdarg.h>
#include "stddef.h"

#ifdef UNITTEST
#define putc	my_putc
#define puts	my_puts
#define getc	my_getc
#define gets	my_gets
#define memset	my_memset
#define memcpy	my_memcpy
#define memmove	my_memmove
#define memcmp	my_memcmp
#define strlen	my_strlen
#define strcpy	my_strcpy
#define strcmp	my_strcmp
#define strncmp	my_strncmp
#define strpbrk	my_strpbrk
#define strtol	my_strtol
#define strtoul	my_strtoul
#define atoi	my_atoi
#define atol	my_atol
#define isdigit	my_isdigit
#define isxdigit	my_isxdigit
#define isspace	my_isspace
#define isprint	my_isprint
#define printf	my_printf
#define sprintf	my_sprintf
#endif

int putc(char c);
int puts(const char *str);
unsigned char getc(void);
char *gets(char *buf, int size);
int putxval(unsigned long value, int column);
int putdval(unsigned int value, int column);
void *memset(void *b, int c, size_t len);
void *memcpy(void *dst, const void *src, size_t len);
void *memmove(void *dst, const void *src, size_t len);
int memcmp(const void *b1, const void *b2, size_t len);
size_t strlen(const char *s);
char *strcpy(char *dst, const char *src);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t len);
char *strpbrk(const char *s, const char *stopset);
long strtol(const char *s, char **endptr, int base);
unsigned long strtoul(const char *s, char **endptr, int base);
int atoi(const char *s);
long atol(const char *s);
int isdigit(int c);
int isxdigit(int c);
int isspace(int c);
int isprint(int c);

int printf(const char *format, ...);
int sprintf(char *buf, const char *format, ...);
int vsprintf(char *buf, const char *format, va_list ap);

void hexdump(const void *buf, size_t size);

#endif /* LIB_H_INCLUDED */
