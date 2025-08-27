
#ifndef PANIC_H
#define PANIC_H

#include <types.h>
#include <tty/tty.h>
#include <util/io.h>

extern void panic(const char *msg);

void __panic(const char *msg, registers_t *regs);

#endif // PANIC_H
