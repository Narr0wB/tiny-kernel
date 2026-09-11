
#include <arch/x86/backtrace.h>
#include <tiny/io.h>
#include <tiny/symbol.h>

void dump_stack_backtrace(void *stack, int log_level)
{
    struct stack_frame *frame = stack;

    for (int frame_idx = 0; frame != NULL; frame = frame->caller, ++frame_idx) {
        const struct symbol *sym = ksym_lookup(frame->ret_addr);

        if (sym) 
            kprintf(log_level, "    [%d][0x%016lx] %s\n", frame_idx, frame->ret_addr, sym->name);
        else 
            kprintf(log_level, "    [%d][0x%016lx] ??\n", frame_idx, frame->ret_addr);
    }
}
