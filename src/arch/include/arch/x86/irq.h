
#ifndef ARCH_X86_IRQ_H
#define ARCH_X86_IRQ_H

#include <tiny/types.h>
#include <tiny/list.h>

struct irq_frame {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rdi, rsi, rbp, rbx, rdx, rcx, rax;

    uint64_t gs, fs;

    uint64_t irqno;
    uint64_t err_code;

    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
};

struct irq_handler {
    const char *id;
    void (*callback)(struct irq_frame *, void *);
    void *data;
    uint32_t flags;
    struct list_head list;
};

#define IRQ_HANDLER_INIT(i, cb, d, f, h) \
    {\
        .id = (i), \
        .callback = (cb), \
        .data = (d), \
        .flags = (f), \
        .list = LIST_NODE_INIT((h).list) \
    }

#endif // ARCH_X86_IRQ_H