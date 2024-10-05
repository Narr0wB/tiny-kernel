
#include <int/int.h>
#include <boot/boot.h>
#include <video/video.h>
#include <tty/tty.h>
#include <util/io.h>
#include <memory/memory.h>
#include <device/device.h>
#include <device/serial.h>

#define KSTACK_BASE 0x700000

int init(bootinfo_t* init_data) {
    init_serial();

    kprintf("Initializing the vga module... ");
    init_video(&init_data->framebuffer);
    kprintf("DONE\n");

    init_tty();

    kprintf("Initializing GDT and kernel memory paging... ");
    init_memory(init_data->map, init_data->kernel_start, init_data->kernel_end);
    kprintf("DONE\n");
    
    kprintf("Initializing IDT and setting up interrupt service routines...  ");
    init_idt();
    kprintf("DONE\n");

    kprintf("Initializing input devices...  ");
    init_device();
    kprintf("DONE\n");

    return 0;
}

__attribute__((aligned(4096))) int _kernel_entry(bootinfo_t* init_data) {
    // Change the kernel stack
    __asm__ volatile ("mov %0, %%rsp" :: "i" (KSTACK_BASE));
    __asm__ volatile ("mov %0, %%rbp" :: "i" (KSTACK_BASE));

    init(init_data);

    int eddu = 0;
    kprintf("I changed the stack, and everything is working, and now the new stack is %p", &eddu);

    while (1) {
        __asm__("hlt");
    }
    
    return 0;
}
