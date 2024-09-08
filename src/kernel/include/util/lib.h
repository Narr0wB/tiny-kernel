
#ifndef STDLIB_H
#define STDLIB_H

#include <common.h>

#define get_bit(v, n) ((v >> n) & 1)
#define set_bit(v, n) ((v) |= (1 << n))
#define clr_bit(v, n) ((v) &= ~(1 << n))

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

#endif // STDLIB_H
