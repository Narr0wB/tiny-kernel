
#ifndef ARCH_X86_PIC_H
#define ARCH_X86_PIC_H

#include <tiny/types.h>

#define PIC1_COMMAND_PORT       0x20
#define PIC2_COMMAND_PORT       0xA0

#define PIC1_DATA_PORT          0x21
#define PIC2_DATA_PORT          0xA1

#define PIC_ICW1_ICW4      0x01
#define PIC_ICW1_SINGLE    0x02
#define PIC_ICW1_INTERVAL4 0x04
#define PIC_ICW1_LEVEL     0x08
#define PIC_ICW1_BASE      0x10

#define PIC_ICW4_X86           0x01
#define PIC_ICW4_AUTO_EOI      0x02
#define PIC_ICW4_BUFFER_MASTER 0x04
#define PIC_ICW4_BUFFER_SLAVE  0x00
#define PIC_ICW4_BUFFERRED     0x08
#define PIC_ICW4_SFNM          0x10

#define PIC_CMD_SPECIFIC_EOI 0x60
#define PIC_CMD_READ_IRR     0x0A
#define PIC_CMD_READ_ISR     0x0B

#define PIC_IRQNO_BASE 0x20
#define PIC_IRQNO_TOP  0x30

void init_pic();
void disable_pic();

void pic_send_eoi(uint8_t irq);
uint16_t pic_read_irq_reg();
uint16_t pic_read_isr_reg();

void pic_disable_irq(uint8_t irq);
void pic_enable_irq(uint8_t irq);

#endif // ARCH_X86_PIC_H