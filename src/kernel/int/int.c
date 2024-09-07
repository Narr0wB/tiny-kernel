
#include <int/int.h>

__attribute__((aligned(0x10))) idt_entry_t IDT[256] = {0};
idt_descriptor_t idt_descriptor = { sizeof(IDT) - 1, (uintptr_t)IDT };

extern void load_idt(idt_descriptor_t *idt_descriptor);
extern void *isr_stub_table[];
extern void *irq_table[];

struct notifier_block *head = NULL;

void init_pic(uint8_t offset1, uint8_t offset2) {
    // Remap the PICs
    outb(PIC1_COMMAND_PORT, PIC_ICW1_BASE | PIC_ICW1_ICW4);
    io_wait();
    outb(PIC2_COMMAND_PORT, PIC_ICW1_BASE | PIC_ICW1_ICW4);
    io_wait();

    outb(PIC1_DATA_PORT, offset1);
    io_wait();
    outb(PIC2_DATA_PORT, offset2);
    io_wait();

    outb(PIC1_DATA_PORT, 0x01 << 2);
    io_wait();
    outb(PIC2_DATA_PORT, 0x02);

    outb(PIC1_DATA_PORT, PIC_ICW4_X86);
    io_wait();
    outb(PIC2_DATA_PORT, PIC_ICW4_X86);
    io_wait();

    outb(PIC1_DATA_PORT, 0x00);
    io_wait();
    outb(PIC2_DATA_PORT, 0x00);
    io_wait();
}

void disable_pic() {
    outb(PIC1_DATA_PORT, 0xFF);
    io_wait();
    outb(PIC2_DATA_PORT, 0xFF);
    io_wait();
}

void pic_send_eoi(uint8_t irq) {
    if (irq >= 8) {
        irq -= 8;
        outb(PIC2_COMMAND_PORT, PIC_CMD_SPECIFIC_EOI | irq);
        io_wait();
        outb(PIC1_COMMAND_PORT, PIC_CMD_SPECIFIC_EOI | 0x02);
        io_wait();
    }
    else {
        outb(PIC1_COMMAND_PORT, PIC_CMD_SPECIFIC_EOI | irq);
        io_wait();
    }
}

uint16_t pic_read_irq_reg() {
    outb(PIC1_COMMAND_PORT, PIC_CMD_READ_IRR);
    io_wait();

    outb(PIC2_COMMAND_PORT, PIC_CMD_READ_IRR);
    io_wait();

    return (inb(PIC2_COMMAND_PORT) | (inb(PIC1_COMMAND_PORT) << 8)); 
}

uint16_t pic_read_isr_reg() {
    outb(PIC1_COMMAND_PORT, PIC_CMD_READ_ISR);
    io_wait();

    outb(PIC2_COMMAND_PORT, PIC_CMD_READ_ISR);
    io_wait();

    return (inb(PIC2_COMMAND_PORT) | (inb(PIC1_COMMAND_PORT) << 8)); 
}

void pic_mask_irq(uint8_t irq) {
    uint8_t port;

    if (irq < 8) {
        port = PIC1_DATA_PORT;
    }
    else {
        irq -= 8;
        port = PIC2_DATA_PORT;
    }

    uint8_t mask = inb(port);
    outb(port, mask | (1 << irq));
    io_wait();
}

void pic_unmask_irq(uint8_t irq) {
    uint8_t port;

    if (irq < 8) {
        port = PIC1_DATA_PORT;
    }
    else {
        irq -= 8;
        port = PIC2_DATA_PORT;
    }

    uint8_t mask = inb(port);
    outb(port, mask & ~(1 << irq));
    io_wait();
}

void init_idt() {
    for (uint8_t interrupt = 0; interrupt < 32; ++interrupt) {
        idt_set_gate(interrupt, (uintptr_t)isr_stub_table[interrupt], 0x08, 0x8E);
    }
    for (uint8_t interrupt = 0; interrupt < 16; ++interrupt) {
        idt_set_gate(interrupt + 0x20, (uintptr_t)irq_table[interrupt], 0x08, 0x8E);
    }

    init_pic(0x20, 0x28);
    pic_mask_irq(0);
    load_idt(&idt_descriptor);
}

void idt_set_gate(uint8_t interrupt, uintptr_t addr, uint16_t segment, uint8_t flags) {
    IDT[interrupt].isr_addr_low     = (addr & 0xFFFF);
    IDT[interrupt].ss               = segment;
    IDT[interrupt].ist_reserved     = 0;
    IDT[interrupt].flags            = flags;
    IDT[interrupt].isr_addr_middle  = (addr >> 16) & 0xFFFF;
    IDT[interrupt].isr_addr_high    = (addr >> 32) & 0xFFFFFFFF;
    IDT[interrupt].reserved         = 0;
}
 
void idt_enable_gate(uint8_t interrupt) {
    set_bit(IDT[interrupt].flags, 8);
}

void idt_disable_gate(uint8_t interrupt) {
    clr_bit(IDT[interrupt].flags, 8);
}

void exception_handler(uint64_t irq, uint64_t err, interrupt_info_t *info, registers_t *regs) {
    char buffer[256];
    sprintf(buffer, "\n\nException no. [0x%lx] occurred with error code: 0x%lx", irq, err);
    panic(buffer);
    __asm__ volatile ("cli; hlt");
}


void irq_handler(uint64_t irq) {
    if (!head) { 
        goto eoi;
    }

    struct notifier_block *current = head;
    while (current) {
        if (irq == current->irq) {
            int result = current->notifier_call(current, irq, NULL);

            if (result != NOTIFY_OK && result != NOTIFY_DONE) {
                panic("Interrupt notifier did not exist successfully");
            }
        }
        current = current->next;
    }
    
eoi:
    pic_send_eoi(irq);
}

// NOTIFIER REGISTERING SYSTEM

void register_notifier_block(struct notifier_block *block) {
    if (!head) {
        head = block;
        return;
    }

    struct notifier_block *current = head;

    while (current->next) {
        current = current->next;
    }
    current->next = block;
}

void unregister_notifier_block(struct notifier_block *block) {
    if (!head) {
        return;
    }

    struct notifier_block *current = head;

    while (current->next && current->next != block) {
        current = current->next;
    }

    if (current->next) {
        current->next = current->next->next;
    }
}
