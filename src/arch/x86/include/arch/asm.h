
#ifndef ARCH_X86_ASM_H
#define ARCH_X86_ASM_H

#include <tiny/types.h>
#include <tiny/compiler.h>

#define get_bit(v, n) ((v >> n) & 1)
#define set_bit(v, n) ((v) |= (1 << n))
#define clr_bit(v, n) ((v) &= ~(1 << n))

static __force_inline void cli() 
{
    __asm__ volatile ("cli");
}

static __force_inline void sti() 
{
    __asm__ volatile ("sti");
}

static __force_inline void hlt() 
{
    __asm__ volatile ("hlt");
}

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ( 
        "outb %b0, %w1" 
        :: "a"(val), "Nd"(port) 
        : "memory"
    );
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ( 
        "inb %w1, %b0"
        : "=a"(ret)
        : "Nd"(port)
        : "memory"
    );
    return ret;
}

static inline void io_wait() {
    outb(0x80, 0);
}

#endif // ARCH_X86_ASM_H