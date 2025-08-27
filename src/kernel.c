
#include <int/int.h>
#include <boot/boot.h>
#include <video/video.h>
#include <tty/tty.h>
#include <util/io.h>
#include <memory/memory.h>
#include <memory/vasl.h>
#include <memory/pmm.h>
#include <device/device.h>
#include <device/serial.h>

#define KSTACK_BASE 0x700000

extern char __stack_end[];
__attribute__((aligned(4096))) int _kernel_entry(struct bootinfo *init_data) {
    // Change the kernel stack
    // __asm__ volatile ("mov %0, %%rsp" :: "r" (__stack_end));
    // __asm__ volatile ("mov %0, %%rbp" :: "r" (__stack_end));

    // init(init_data);

    init_serial();

    kprintf("Initializing the vga and tty module...");
    init_video(&init_data->framebuffer);
    kprintf("DONE\n");

    init_tty();

    kprintf("Initializing GDT and kernel memory paging... ");
    init_memory(init_data);
    kprintf("DONE\n");
    
    kprintf("Initializing IDT and setting up interrupt service routines...");
    init_idt();
    kprintf("DONE\n");

    kprintf("Initializing input devices...");
    init_device();
    kprintf("DONE\n");

    extern struct memory_info _mem_info;

    int test = 0;
    kprintf("[INFO] Working on stack %p"EOL, &test);
    kprintf("[INFO] Available Physical RAM: %d pages, %dM"EOL, _mem_info.available_phys_pages, (_mem_info.available_phys_pages * PAGE_SIZE) / (1024 * 1024));
    kprintf("[INFO] eddu %p"EOL, init_data->framebuffer.base_addr);

    while (1) {
        __asm__("hlt");
    }
    
    return 0;
}

