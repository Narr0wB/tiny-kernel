
#ifndef PANIC_H
#define PANIC_H

#include <tiny/types.h>
#include <tiny/tty/tty.h>
#include <tiny/io.h>

extern void panic(const char *msg);

void __panic(const char *msg, registers_t *regs);

#endif // PANIC_H
