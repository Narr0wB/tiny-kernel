#include <arch/x86/idt.h>
#include <arch/x86/asm.h>
#include <arch/x86/drivers/pic.h>
#include <arch/x86/timer.h>

#define PIT_CHANNEL0_PORT 0x40
#define PIT_COMMAND_PORT  0x43
#define PIT_MODE_SQUARE   0x36
#define PIT_BASE_FREQ     1193182
#define TIMER_TARGET_FREQ 1000

static volatile u64 timer_ms = 0;

static void timer_handler(struct irq_frame *frame, void *ctx)
{
    timer_ms++;
    pic_send_eoi(0);
}

static struct irq_handler timer = IRQ_HANDLER_INIT("1kHz timer", timer_handler, NULL, 0, timer);

u64 timer_get_ms()
{
    return timer_ms;
}

void init_timer()
{
    u16 divisor = (u16)(PIT_BASE_FREQ / TIMER_TARGET_FREQ);

    outb(PIT_COMMAND_PORT, PIT_MODE_SQUARE);
    outb(PIT_CHANNEL0_PORT, (u8)(divisor & 0xFF));
    outb(PIT_CHANNEL0_PORT, (u8)((divisor >> 8) & 0xFF));

    register_irq_handler(0x20, &timer);
}