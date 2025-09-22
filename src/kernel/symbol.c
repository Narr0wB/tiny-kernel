
#include <tiny/symbol.h>
#include <tiny/compiler.h>
#include <arch/idt.h>

extern struct symbol ksyms[] __weak;
extern size_t ksyms_len __weak;

struct symbol *ksym_lookup(uintptr_t ret_addr)
{
    for (size_t i = 0; i < ksyms_len; ++i) {
        struct symbol *sym = &(ksyms[i]); 
        if (ret_addr >= sym->addr && ret_addr <= (sym->addr + sym->size))
            return sym;
    }

    return NULL;
}