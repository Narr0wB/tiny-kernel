
#ifndef INT_H
#define INT_H

#include <common.h>
#include <klibc/lib.h>
#include <klibc/io.h>
#include <int/isr.h>

typedef struct {
    uint16_t isr_addr_low;
    uint16_t ss;
    uint8_t ist_reserved;
    uint8_t flags;
    uint16_t isr_addr_middle;
    uint32_t isr_addr_high;
    uint32_t reserved;
} __attribute__((packed)) idt_entry_t;

typedef struct {
    uint16_t size;
    uint64_t addr;
} __attribute__((packed)) idt_descriptor_t;

typedef enum {
    IDT_FLAGS_GATE_INT          = 0xE,
    IDT_FLAGS_GATE_TRP          = 0xF,

    IDT_FLAGS_DPL_RING0         = 0x0 << 5,
    IDT_FLAGS_DPL_RING1         = 0x1 << 5,
    IDT_FLAGS_DPL_RING2         = 0x2 << 5,
    IDT_FLAGS_DPL_RING3         = 0x3 << 5,

    IDT_FLAG_PRESENT            = 0x80
} IDT_FLAGS;

void init_idt();
void init_pic(uint8_t offset);

void idt_set_gate(uint8_t interrupt, uintptr_t addr, uint16_t segment, uint8_t flags);
void idt_enable_gate(uint8_t interrupt);
void idt_disable_gate(uint8_t interrupt);

#endif // INT_H
