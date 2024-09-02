
#include <panic.h>

void __panic(const char *msg, registers_t *regs) {
    tty_clear(CLEAR_COLOR);
    tty_set_cursor((cursor_t){0, 0});

    kprintf(" /$$$$$$$   /$$$$$$  /$$   /$$ /$$$$$$  /$$$$$$  /$$"EOL);
    kprintf("| $$__  $$ /$$__  $$| $$$ | $$|_  $$_/ /$$__  $$| $$"EOL);
    kprintf("| $$  \\ $$| $$  \\ $$| $$$$| $$  | $$  | $$  \\__/| $$"EOL);
    kprintf("| $$$$$$$/| $$$$$$$$| $$ $$ $$  | $$  | $$      | $$"EOL);
    kprintf("| $$____/ | $$__  $$| $$  $$$$  | $$  | $$      |__/"EOL);
    kprintf("| $$      | $$  | $$| $$\\  $$$  | $$  | $$    $$    "EOL);
    kprintf("| $$      | $$  | $$| $$ \\  $$ /$$$$$$|  $$$$$$/ /$$"EOL);
    kprintf("|__/      |__/  |__/|__/  \\__/|______/ \\______/ |__/"EOL);

    kprintf("\n\nREGISTERS:"EOL);
    kprintf("RAX: 0x%16llx"EOL, regs->rax);
    kprintf("RCX: 0x%16llx"EOL, regs->rcx);
    kprintf("RDX: 0x%16llx"EOL, regs->rdx);
    kprintf("RBX: 0x%16llx"EOL, regs->rbx);
    kprintf("RBP: 0x%16llx"EOL, regs->rbp);
    kprintf("RSI: 0x%16llx"EOL, regs->rsi);
    kprintf("RDI: 0x%16llx"EOL, regs->rdi);

    kprintf("\n\nCause of kernel panic: %s", msg);

    while (1) {
        __asm__("hlt");
    } 
}
