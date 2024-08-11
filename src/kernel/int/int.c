
#include <int/int.h>

__attribute__((aligned(0x10))) idt_entry_t IDT[256];
idt_descriptor_t idt_descriptor = { sizeof(IDT) - 1, (uintptr_t)IDT };

extern void load_idt(idt_descriptor_t *idt_descriptor);
extern void *isr_stub_table[];

void init_idt() {
    for (uint8_t interrupt = 0; interrupt < 32; ++interrupt) {
        idt_set_gate(interrupt, (uintptr_t)isr_stub_table[interrupt], 0x08, 0x8E);
    }

    // Remap the PIC 
    outb(0x20, 0x11); 
    outb(0xA0, 0x11); 
    outb(0x21, 0x20); 
    outb(0xA1, 0x28);
    outb(0x21, 0x04);
    outb(0xA1, 0x02); 
    outb(0x21, 0x01); 
    outb(0xA1, 0x01); 
    outb(0x21, 0x0); 
    outb(0xA1, 0x0); 

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
