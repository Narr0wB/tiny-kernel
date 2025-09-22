
#ifndef ARCH_X86_BACKTRACE_H
#define ARCH_X86_BACKTRACE_H

#include <tiny/types.h>
#include <tiny/compiler.h>

void dump_stack_backtrace(void *stack, int log_level);

/* The pointer in caller is just the RBP of the caller frame */
struct stack_frame {
    struct stack_frame *caller;
    uintptr_t ret_addr;
} __packed;

#endif // ARCH_X86_BACKTRACE_H