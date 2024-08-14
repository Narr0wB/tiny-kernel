
#ifndef IO_H
#define IO_H

#include <common.h>
#include <stdarg.h>
#include <klibc/string.h>
#include <tty/tty.h>

#define EOL "\n"

int putchar(int c);
int puts(const char *str);
int kprintf(const char *fmt, ...);

int vsprintf(void *pc, void *ps, const char *fmt, va_list args);

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ( "outb %b0, %w1" : : "a"(val), "Nd"(port) : "memory");
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ( "inb %w1, %b0"
                   : "=a"(ret)
                   : "Nd"(port)
                   : "memory");
    return ret;
}

static inline void io_wait(void) {
    outb(0x80, 0);
}

#endif // IO_H
