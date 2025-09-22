
#ifndef SYMBOL_H
#define SYMBOL_H

#include <tiny/types.h>

struct symbol {
    const char *name;
    const void *addr;
    size_t size;
};

struct symbol *ksym_lookup(uintptr_t addr);

#endif // SYMBOL_H