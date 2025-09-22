
#include <arch/drivers/pic.h>
#include <arch/asm.h>

void init_pic() 
{
    // Remap the PICs
    outb(PIC1_COMMAND_PORT, PIC_ICW1_BASE | PIC_ICW1_ICW4);
    io_wait();
    outb(PIC2_COMMAND_PORT, PIC_ICW1_BASE | PIC_ICW1_ICW4);
    io_wait();

    outb(PIC1_DATA_PORT, PIC_IRQNO_BASE);
    io_wait();
    outb(PIC2_DATA_PORT, PIC_IRQNO_BASE + 8);
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

void disable_pic() 
{
    outb(PIC1_DATA_PORT, 0xFF);
    io_wait();
    outb(PIC2_DATA_PORT, 0xFF);
    io_wait();
}

void pic_send_eoi(uint8_t irq) 
{
    if (irq < 8) {
        outb(PIC1_COMMAND_PORT, PIC_CMD_SPECIFIC_EOI | irq);
        io_wait();
    } else {
        irq -= 8;
        outb(PIC2_COMMAND_PORT, PIC_CMD_SPECIFIC_EOI | irq);
        io_wait();
        outb(PIC1_COMMAND_PORT, PIC_CMD_SPECIFIC_EOI | 0x02);
        io_wait();
    }
}

uint16_t pic_read_irq_reg() 
{
    outb(PIC1_COMMAND_PORT, PIC_CMD_READ_IRR);
    io_wait();

    outb(PIC2_COMMAND_PORT, PIC_CMD_READ_IRR);
    io_wait();

    return (inb(PIC2_COMMAND_PORT) | (inb(PIC1_COMMAND_PORT) << 8)); 
}

uint16_t pic_read_isr_reg() 
{
    outb(PIC1_COMMAND_PORT, PIC_CMD_READ_ISR);
    io_wait();

    outb(PIC2_COMMAND_PORT, PIC_CMD_READ_ISR);
    io_wait();

    return (inb(PIC2_COMMAND_PORT) | (inb(PIC1_COMMAND_PORT) << 8)); 
}

void pic_disable_irq(uint8_t pic_irq) 
{
    uint8_t port;

    if (pic_irq < 8) {
        port = PIC1_DATA_PORT;
    }
    else {
        pic_irq -= 8;
        port = PIC2_DATA_PORT;
    }

    uint8_t mask = inb(port);
    outb(port, mask | (1 << pic_irq));
    io_wait();
}

void pic_enable_irq(uint8_t pic_irq) 
{
    uint8_t port;

    if (pic_irq < 8) {
        port = PIC1_DATA_PORT;
    }
    else {
        pic_irq -= 8;
        port = PIC2_DATA_PORT;
    }

    uint8_t mask = inb(port);
    outb(port, mask & ~(1 << pic_irq));
    io_wait();
}