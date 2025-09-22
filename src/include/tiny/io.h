
#ifndef IO_H
#define IO_H

#include <stdarg.h>
#include <tiny/types.h>
#include <tiny/tty/tty.h>
#include <tiny/serial.h>

typedef int (*putc_fn_t)(char);
typedef int (*puts_fn_t)(const char*);

#define KERN_ERROR   0
#define KERN_WARNING 1
#define KERN_INFO    2
#define KERN_DEBUG   3
#define KERN_TRACE   4
#define KERN_NONE    5

int vsprintf(putc_fn_t pc, puts_fn_t ps, const char *fmt, va_list args);
int sprintf(char *buffer, const char *fmt, ...);

int kprintf(int level, const char *fmt, ...);

#endif // IO_H
