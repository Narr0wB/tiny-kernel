
#include <tiny/device/keyboard.h>
#include <tiny/io.h>
#include <tiny/notifier.h>
#include <tiny/panic.h>
#include <tiny/mm/memory.h>

uint8_t key_buffer[10];
size_t buffer_index;
keycode_t state;

int keyboard_notifier(struct notifier_block *b, uint64_t a, void *d) {
    uint8_t scan_code = ps2_read_data();
    kprintf(KERN_INFO, "scan_code = %d"EOL, scan_code);

    // VirtualKey key = get_vk(scan_code_set, scan_code); 
    // bool is_pressed = get_state(scan_code_set, scan_code);
    // state.key = (uint8_t)key;
    //
    // if (is_modifier(key) && is_pressed) {
    //     state.modifiers |= (key & modifier_mask);
    //     return NOTIFY_OK;
    // }
    // else if (is_modifier(key) && !is_pressed) {
    //     state.modifiers &= ~(key & modifier_mask); 
    // }
    // else if (!is_modifier(key) && is_pressed && state.modifiers) {
    //     keycode_t code = { .key = (uint8_t)key, .modifiers = state.modifiers };
    //     
    // }
    panic("odio zingari!");
    
    return NOTIFY_OK;
}

void init_keyboard() {
    // struct notifier_block *block = kpalloc(); 
    // block->notifier_call = keyboard_notifier; 
    // block->next = NULL;
    // block->irq = 1;

    // register_notifier_block(block); 
}
