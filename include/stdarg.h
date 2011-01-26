#ifndef STDARG_H_INCLUDED
#define STDARG_H_INCLUDED


#define STDARG_ALIGN(s)		(((s) + 3) & -4)

typedef char *va_list;

#define va_start(ap, last_arg)	\
	((ap) = ((char *)&(last_arg) + STDARG_ALIGN(sizeof(last_arg))))

#define va_arg(ap, type)	\
	((ap) += STDARG_ALIGN(sizeof(type)), *(type *)((ap) - sizeof(type)))

#define va_end(ap)

#endif /* STDARG_H_INCLUDED */
