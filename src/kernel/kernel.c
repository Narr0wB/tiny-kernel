
#include <int/int.h>
#include <boot/boot.h>
#include <video/video.h>
#include <klibc/io.h>
#include <memory/memory.h>

int init(bootinfo_t* init_data) {
    init_video(&init_data->framebuffer);
    init_mm();
    init_idt();

    return 0;
}

int _kernel_entry(void* init_data) {
    init((bootinfo_t*)init_data);
    kprintf("hello world!\n");

    while (1) {
        __asm__("hlt");
    }
    
    return 0;
}
