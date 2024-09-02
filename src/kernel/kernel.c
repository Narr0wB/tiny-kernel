
#include <int/int.h>
#include <boot/boot.h>
#include <video/video.h>
#include <tty/tty.h>
#include <util/io.h>
#include <memory/memory.h>

int init(bootinfo_t* init_data) {
    init_video(&init_data->framebuffer);
    init_tty();
    
    kprintf("Initializing GDT and memory manager...  ");
    init_mm();
    kprintf("DONE\n");

    kprintf("Initializing IDT and setting up interrupt service routines...  ");
    init_idt();
    kprintf("DONE\n");

    return 0;
}

extern console_t console;
int _kernel_entry(void* init_data) {
    init((bootinfo_t*)init_data);
    
    for (int i = 0; i < 10; i++)
        kprintf("test %% %d %d %d"EOL, ((bootinfo_t*)init_data)->framebuffer.height, ((bootinfo_t*)init_data)->framebuffer.len_scanline, ((bootinfo_t*)init_data)->framebuffer.size);

    tty_show_prompt();
    while (1) {
        __asm__("hlt");
    }
    
    return 0;
}
