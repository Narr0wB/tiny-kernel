
#include <tiny/panic.h>

void __panic(const char *msg, registers_t *regs) 
{
    tty_clear(CLEAR_COLOR);
    tty_set_cursor((cursor_t){0, 0});
    
    kprintf(KERN_ERROR, "\n\n");
    kprintf(KERN_ERROR, " /$$$$$$$   /$$$$$$  /$$   /$$ /$$$$$$  /$$$$$$  /$$"EOL);
    kprintf(KERN_ERROR, "| $$__  $$ /$$__  $$| $$$ | $$|_  $$_/ /$$__  $$| $$"EOL);
    kprintf(KERN_ERROR, "| $$  \\ $$| $$  \\ $$| $$$$| $$  | $$  | $$  \\__/| $$"EOL);
    kprintf(KERN_ERROR, "| $$$$$$$/| $$$$$$$$| $$ $$ $$  | $$  | $$      | $$"EOL);
    kprintf(KERN_ERROR, "| $$____/ | $$__  $$| $$  $$$$  | $$  | $$      |__/"EOL);
    kprintf(KERN_ERROR, "| $$      | $$  | $$| $$\\  $$$  | $$  | $$    $$    "EOL);
    kprintf(KERN_ERROR, "| $$      | $$  | $$| $$ \\  $$ /$$$$$$|  $$$$$$/ /$$"EOL);
    kprintf(KERN_ERROR, "|__/      |__/  |__/|__/  \\__/|______/ \\______/ |__/"EOL);

    kprintf(KERN_ERROR, "\n\nREGISTERS:"EOL);
    kprintf(KERN_ERROR, "RAX: 0x%16llx"EOL, regs->rax);
    kprintf(KERN_ERROR, "RCX: 0x%16llx"EOL, regs->rcx);
    kprintf(KERN_ERROR, "RDX: 0x%16llx"EOL, regs->rdx);
    kprintf(KERN_ERROR, "RBX: 0x%16llx"EOL, regs->rbx);
    kprintf(KERN_ERROR, "RBP: 0x%16llx"EOL, regs->rbp);
    kprintf(KERN_ERROR, "RSP: 0x%16llx"EOL, regs->rsp);
    kprintf(KERN_ERROR, "RSI: 0x%16llx"EOL, regs->rsi);
    kprintf(KERN_ERROR, "RDI: 0x%16llx"EOL, regs->rdi);

    kprintf(KERN_ERROR, "\n\nCause of kernel panic: %s", msg);

    while (1) {
        __asm__("hlt");
    } 
}
