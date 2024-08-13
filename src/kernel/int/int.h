
#ifndef INT_H
#define INT_H

#include <common.h>
#include <klibc/lib.h>
#include <klibc/io.h>

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

void idt_set_gate(uint8_t interrupt, uintptr_t addr, uint16_t segment, uint8_t flags);
void idt_enable_gate(uint8_t interrupt);
void idt_disable_gate(uint8_t interrupt);

// PIC inherent stuff
#define PIC1_COMMAND_PORT       0x20
#define PIC2_COMMAND_PORT       0xA0

#define PIC1_DATA_PORT          0x21
#define PIC2_DATA_PORT          0xA1

typedef enum {
    PIC_ICW1_ICW4               = 0x01,
    PIC_ICW1_SINGLE             = 0x02,
    PIC_ICW1_INTERVAL4          = 0x04,
    PIC_ICW1_LEVEL              = 0x08,
    PIC_ICW1_BASE               = 0x10
} PIC_ICW1;

typedef enum {
    PIC_ICW4_X86                = 0x01,
    PIC_ICW4_AUTO_EOI           = 0x02,
    PIC_ICW4_BUFFER_MASTER      = 0x04,
    PIC_ICW4_BUFFER_SLAVE       = 0x00,
    PIC_ICW4_BUFFERRED          = 0x08,
    PIC_ICW4_SFNM               = 0x10
} PIC_ICW4;

typedef enum {
    PIC_CMD_SPECIFIC_EOI        = 0x60,
    PIC_CMD_READ_IRR            = 0x0A,
    PIC_CMD_READ_ISR            = 0x0B
} PIC_CMD;

void init_pic(uint8_t offset1, uint8_t offset2);
void disable_pic();

void pic_send_eoi(uint8_t irq);
uint16_t pic_read_irq_reg();
uint16_t pic_read_isr_reg();

void pic_mask_irq(uint8_t irq);
void pic_unmask_irq(uint8_t irq);

__attribute__((noreturn)) void isr_handler(void);

#endif // INT_H
