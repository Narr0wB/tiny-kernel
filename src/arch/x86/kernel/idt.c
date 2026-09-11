
#include <arch/x86/gdt.h>
#include <arch/x86/irq.h>
#include <arch/x86/drivers/pic.h>
#include <arch/x86/asm.h>
#include <arch/x86/backtrace.h>

#include <tiny/io.h>

#include <arch/x86/idt.h>

static struct idt_entry idt[256] = {0};
static struct idt_info  info[256] = {0};

static const char *cpu_exceptions[32] = {
    [0]  = "Divide-by-zero (#DE)",
    [1]  = "Debug (#DB)",
    [2]  = "Non-maskable interrupt (NMI)",
    [3]  = "Breakpoint (#BP)",
    [4]  = "Overflow (#OF)",
    [5]  = "BOUND range exceeded (#BR)",
    [6]  = "Invalid opcode (#UD)",
    [7]  = "Device not available (#NM)",
    [8]  = "Double fault (#DF)",
    [9]  = "Coprocessor segment overrun (reserved)",
    [10] = "Invalid TSS (#TS)",
    [11] = "Segment not present (#NP)",
    [12] = "Stack-segment fault (#SS)",
    [13] = "General protection fault (#GP)",
    [14] = "Page fault (#PF)",
    [15] = "Reserved",
    [16] = "x87 FPU floating-point error (#MF)",
    [17] = "Alignment check (#AC)",
    [18] = "Machine check (#MC)",
    [19] = "SIMD floating-point exception (#XM/#XF)",
    [20] = "Virtualization exception (#VE)",
    [21] = "Control-protection exception (#CP)",
    [22] = "Reserved",
    [23] = "Reserved",
    [24] = "Reserved",
    [25] = "Reserved",
    [26] = "Reserved",
    [27] = "Reserved",
    [28] = "Hypervisor injection exception (AMD)",
    [29] = "VMM communication exception (AMD #VC)",
    [30] = "Security exception (Intel #SX)",
    [31] = "Reserved",
};

static void unhandled_cpu_exception_cb(struct irq_frame *frame, void *data)
{
    kprintf(KERN_ERROR, "FATAL: Got an unhandled CPU exception! Exception: %s (no. %d), Err: %d\n", cpu_exceptions[frame->irqno], frame->irqno, frame->err_code);

    kprintf(KERN_ERROR, "RAX: 0x%016lx, RBX: 0x%016lx\n", frame->rax, frame->rbx);
    kprintf(KERN_ERROR, "RCX: 0x%016lx, RDX: 0x%016lx\n", frame->rcx, frame->rdx);
    kprintf(KERN_ERROR, "RSI: 0x%016lx, RDI: 0x%016lx\n", frame->rsi, frame->rdi);
    kprintf(KERN_ERROR, "RSP: 0x%016lx, RBP: 0x%016lx\n", frame->rsp, frame->rbp);

    kprintf(KERN_ERROR, "CS: 0x%04x, SS: 0x%04x\n", frame->cs, frame->ss);
    kprintf(KERN_ERROR, "GS: 0x%04x, FS: 0x%04x\n", frame->gs, frame->fs);

    kprintf(KERN_ERROR, "Current RIP: %016lx\n", frame->rip);
    kprintf(KERN_ERROR, "Stack backtrace: \n");
    dump_stack_backtrace((void *)frame->rbp, KERN_ERROR);

    while (1) {
        hlt();
    }
}

static struct irq_handler cpu_exceptions_handlers[32] = {
    [0]  = IRQ_HANDLER_INIT("Divide-by-zero (#DE)",                 unhandled_cpu_exception_cb, NULL, 0, cpu_exceptions_handlers[0]),
    [1]  = IRQ_HANDLER_INIT("Debug (#DB)",                          unhandled_cpu_exception_cb, NULL, 0, cpu_exceptions_handlers[1]),
    [2]  = IRQ_HANDLER_INIT("Non-maskable interrupt (NMI)",         unhandled_cpu_exception_cb, NULL, 0, cpu_exceptions_handlers[2]),
    [3]  = IRQ_HANDLER_INIT("Breakpoint (#BP)",                     unhandled_cpu_exception_cb, NULL, 0, cpu_exceptions_handlers[3]),
    [4]  = IRQ_HANDLER_INIT("Overflow (#OF)",                       unhandled_cpu_exception_cb, NULL, 0, cpu_exceptions_handlers[4]),
    [5]  = IRQ_HANDLER_INIT("BOUND range exceeded (#BR)",           unhandled_cpu_exception_cb, NULL, 0, cpu_exceptions_handlers[5]),
    [6]  = IRQ_HANDLER_INIT("Invalid opcode (#UD)",                 unhandled_cpu_exception_cb, NULL, 0, cpu_exceptions_handlers[6]),
    [7]  = IRQ_HANDLER_INIT("Device not available (#NM)",           unhandled_cpu_exception_cb, NULL, 0, cpu_exceptions_handlers[7]),
    [8]  = IRQ_HANDLER_INIT("Double fault (#DF)",                   unhandled_cpu_exception_cb, NULL, 0, cpu_exceptions_handlers[8]),
    [9]  = IRQ_HANDLER_INIT("Coprocessor segment overrun (reserved)", unhandled_cpu_exception_cb, NULL, 0, cpu_exceptions_handlers[9]),
    [10] = IRQ_HANDLER_INIT("Invalid TSS (#TS)",                    unhandled_cpu_exception_cb, NULL, 0, cpu_exceptions_handlers[10]),
    [11] = IRQ_HANDLER_INIT("Segment not present (#NP)",            unhandled_cpu_exception_cb, NULL, 0, cpu_exceptions_handlers[11]),
    [12] = IRQ_HANDLER_INIT("Stack-segment fault (#SS)",            unhandled_cpu_exception_cb, NULL, 0, cpu_exceptions_handlers[12]),
    [13] = IRQ_HANDLER_INIT("General protection fault (#GP)",       unhandled_cpu_exception_cb, NULL, 0, cpu_exceptions_handlers[13]),
    [15] = IRQ_HANDLER_INIT("Reserved",                             unhandled_cpu_exception_cb, NULL, 0, cpu_exceptions_handlers[15]),
    [16] = IRQ_HANDLER_INIT("x87 FPU floating-point error (#MF)",   unhandled_cpu_exception_cb, NULL, 0, cpu_exceptions_handlers[16]),
    [17] = IRQ_HANDLER_INIT("Alignment check (#AC)",                unhandled_cpu_exception_cb, NULL, 0, cpu_exceptions_handlers[17]),
    [18] = IRQ_HANDLER_INIT("Machine check (#MC)",                  unhandled_cpu_exception_cb, NULL, 0, cpu_exceptions_handlers[18]),
    [19] = IRQ_HANDLER_INIT("SIMD floating-point exception (#XM/#XF)", unhandled_cpu_exception_cb, NULL, 0, cpu_exceptions_handlers[19]),
    [20] = IRQ_HANDLER_INIT("Virtualization exception (#VE)",       unhandled_cpu_exception_cb, NULL, 0, cpu_exceptions_handlers[20]),
    [21] = IRQ_HANDLER_INIT("Control-protection exception (#CP)",   unhandled_cpu_exception_cb, NULL, 0, cpu_exceptions_handlers[21]),
    [22] = IRQ_HANDLER_INIT("Reserved",                             unhandled_cpu_exception_cb, NULL, 0, cpu_exceptions_handlers[22]),
    [23] = IRQ_HANDLER_INIT("Reserved",                             unhandled_cpu_exception_cb, NULL, 0, cpu_exceptions_handlers[23]),
    [24] = IRQ_HANDLER_INIT("Reserved",                             unhandled_cpu_exception_cb, NULL, 0, cpu_exceptions_handlers[24]),
    [25] = IRQ_HANDLER_INIT("Reserved",                             unhandled_cpu_exception_cb, NULL, 0, cpu_exceptions_handlers[25]),
    [26] = IRQ_HANDLER_INIT("Reserved",                             unhandled_cpu_exception_cb, NULL, 0, cpu_exceptions_handlers[26]),
    [27] = IRQ_HANDLER_INIT("Reserved",                             unhandled_cpu_exception_cb, NULL, 0, cpu_exceptions_handlers[27]),
    [28] = IRQ_HANDLER_INIT("Hypervisor injection exception (AMD)", unhandled_cpu_exception_cb, NULL, 0, cpu_exceptions_handlers[28]),
    [29] = IRQ_HANDLER_INIT("VMM communication exception (AMD #VC)",unhandled_cpu_exception_cb, NULL, 0, cpu_exceptions_handlers[29]),
    [30] = IRQ_HANDLER_INIT("Security exception (Intel #SX)",       unhandled_cpu_exception_cb, NULL, 0, cpu_exceptions_handlers[30]),
    [31] = IRQ_HANDLER_INIT("Reserved",                             unhandled_cpu_exception_cb, NULL, 0, cpu_exceptions_handlers[31]),
};

extern uintptr_t irq_hnd_table[];

static inline void idt_load(struct idt_entry *idt, size_t size) {
    static struct idt_ptr ptr;

    ptr.size = size;
    ptr.addr = (uintptr_t)idt;

    __asm__ volatile (
        "movq %0, %%rax\n"
        "lidt (%%rax)\n"
        :: "r" (&ptr)
        : "memory", "rax"
    );
}

void register_irq_handler(int irqno, struct irq_handler *hand)
{
    struct idt_info *i = info + irqno;

    /* Temporarily block any maskable-interrupt requets when modifying the idt_info structure */
    cli();

    list_add_tail(&hand->list, &i->list);

    /* If we are attaching a handler to an IRQ number of the PIC, then enable that PIC IRQ */ 
    if (irqno >= PIC_IRQNO_BASE && irqno <= PIC_IRQNO_TOP) 
        pic_enable_irq(irqno - PIC_IRQNO_BASE);

    sti();

    kprintf(KERN_INFO, "Registered irq handler to irqno %d, name: %s\n", irqno, hand->id);
}

void irq_global_handler(struct irq_frame *frame) {
    struct idt_info *i = info + frame->irqno;
    int pic_irq = -1;

    atomic_inc(&i->count);

    kprintf(KERN_DEBUG, "Got irq no. %lx with error code %lx, cs %ld, rip %lx, rsp %lx\n", frame->irqno, frame->err_code, frame->cs, frame->rip, frame->rsp);

    if (frame->irqno >= PIC_IRQNO_BASE && frame->irqno <= PIC_IRQNO_TOP) {
        pic_irq = frame->irqno - PIC_IRQNO_BASE;

        pic_disable_irq(pic_irq);
        pic_send_eoi(pic_irq);
    }

    /* Dispatch any callbacks tied to this particular irq */
    struct irq_handler *hand;
    list_foreach_entry(&i->list, hand, list)
        hand->callback(frame, hand->data);
    
    if (pic_irq > 0)
        pic_enable_irq(pic_irq);
}

void init_idt() {
    for (int i = 0; i < 256; ++i) {
        list_head_init(&info[i].list);
        IDT_SET_ENTRY(idt[i], (uint64_t)irq_hnd_table[i], GDT_ENTRY_KERNEL_CS_OFF, 0, IRQ_TYPE_INTERRUPT, DPL_KERNEL);
    }

    for (int i = 0; i < 32; ++i) {
        if (cpu_exceptions_handlers[i].callback)
            register_irq_handler(i, &cpu_exceptions_handlers[i]);
    }
    

    idt_load(idt, sizeof(idt) - 1);
}
