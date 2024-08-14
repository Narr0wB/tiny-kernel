
#include <int/int.h>
#include <boot/boot.h>
#include <video/video.h>
#include <klibc/io.h>
#include <memory/memory.h>

int init(bootinfo_t* init_data) {
    init_video(&init_data->framebuffer);
    
    kprintf("Initializing GDT and memory manager...  ");
    init_mm();
    kprintf("DONE\n");

    kprintf("Initializing IDT and setting up interrupt service routines...  ");
    init_idt();
    kprintf("DONE\n");

    kprintf("\n\n");
    return 0;
}

int _kernel_entry(void* init_data) {
    init((bootinfo_t*)init_data);
    kprintf("test %% %ld %x", 38947928, 349834);

    while (1) {
        __asm__("hlt");
    }
    
    return 0;
}
