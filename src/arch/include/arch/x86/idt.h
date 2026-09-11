
#ifndef ARCH_X86_IDT_H
#define ARCH_X86_IDT_H

#include <tiny/types.h>
#include <tiny/compiler.h>
#include <tiny/list.h>
#include <arch/x86/irq.h>
#include <arch/x86/atomic.h>

struct idt_entry {
    uint16_t offset_low;
    uint16_t cs; 
    uint8_t ist     :3;
    uint8_t resvd1  :5;
    uint8_t type    :4;
    uint8_t resvd2  :1;
    uint8_t dpl     :2;
    uint8_t present :1;
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t resvd3;
} __packed;

struct idt_info {
    atomic_t count;
    uint32_t flags;

    struct list_head list;
};

struct idt_ptr {
    uint16_t size;
    uint64_t addr;
} __packed;

#define IRQ_TYPE_INTERRUPT 0xE
#define IRQ_TYPE_TRAP      0xF

#define IDT_SET_ENTRY(entry, handler, _cs, _ist, _type, _dpl) do { \
    (entry).offset_low = (handler & 0xFFFF); \
    (entry).cs = _cs; \
    (entry).ist = _ist; \
    (entry).resvd1 = 0; \
    (entry).type = _type; \
    (entry).resvd2 = 0; \
    (entry).dpl = _dpl; \
    (entry).present = 1; \
    (entry).offset_mid = ((handler >> 16) & 0xFFFF); \
    (entry).offset_high = ((handler >> 32) & 0xFFFFFFFF); \
    (entry).resvd3 = 0; \
    } while (0)

void register_irq_handler(int irqno, struct irq_handler *hand);
void init_idt();

#endif // ARCH_X86_IDT_H
