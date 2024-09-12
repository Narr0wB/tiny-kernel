
#include <int/int.h>
#include <boot/boot.h>
#include <video/video.h>
#include <tty/tty.h>
#include <util/io.h>
#include <memory/memory.h>
#include <device/device.h>
#include <device/serial.h>

int init(bootinfo_t* init_data) {
    init_serial();
    init_video(&init_data->framebuffer);
    init_tty();
    init_memory(init_data->map, init_data->kernel_start, init_data->kernel_end);
    
    kprintf("Initializing IDT and setting up interrupt service routines...  ");
    init_idt();
    kprintf("DONE\n");

    kprintf("Initializing input devices...  ");
    init_device();
    kprintf("DONE\n");

    return 0;
}

__attribute__((aligned(4096))) int _kernel_entry(bootinfo_t* init_data) {
    init(init_data);
    
    while (1) {
        __asm__("hlt");
    }
    
    return 0;
}
