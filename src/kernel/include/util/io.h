
#ifndef IO_H
#define IO_H

#include <common.h>
#include <stdarg.h>
#include <tty/tty.h>
#include <device/serial.h>

typedef int (*putc_fn_t)(char);
typedef int (*puts_fn_t)(const char*);

// USER OUTPUT

int vsprintf(putc_fn_t pc, puts_fn_t ps, const char *fmt, va_list args);
int sprintf(char *buffer, const char *fmt, ...);

int kprintf(const char *fmt, ...);

#endif // IO_H
