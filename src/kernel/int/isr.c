
#include <int/isr.h>

void isr_handler() {
    __asm__ volatile ("cli; hlt");
}
