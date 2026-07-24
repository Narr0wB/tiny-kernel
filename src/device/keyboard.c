
#include <arch/idt.h>
#include <arch/drivers/ps2.h>

#include <tiny/io.h>
#include <tiny/notifier.h>
#include <tiny/panic.h>
#include <tiny/mm/memory.h>
#include <tiny/mm/palloc.h>

#include <tiny/device/keyboard.h>

uint8_t key_buffer[10];
size_t buffer_index;
keycode_t state;

void keyboard_notifier(struct irq_frame *frame, void *data) 
{
    uint8_t scan_code = ps2_read_data();
    kprintf(KERN_INFO, "scan_code = %d"EOL, scan_code);

    panic("odio zingari!");
}

void init_keyboard() 
{
    struct irq_handler *block = (struct irq_handler*)palloc(0, 0xEDD0);
    block->id       = "keyboard_notifier";
    block->callback = keyboard_notifier;
    block->data     = NULL;
    block->flags    = 0;
    
    register_irq_handler(1, block);
}