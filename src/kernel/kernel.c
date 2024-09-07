
#include <int/int.h>
#include <boot/boot.h>
#include <video/video.h>
#include <tty/tty.h>
#include <util/io.h>
#include <memory/memory.h>
#include <device/device.h>

int init(bootinfo_t* init_data) {
    init_video(&init_data->framebuffer);
    init_tty();
    init_memory(init_data->map);
    
    kprintf("Initializing IDT and setting up interrupt service routines...  ");
    init_idt();
    kprintf("DONE\n");

    kprintf("Initializing input devices...  ");
    init_device();
    kprintf("DONE\n");

    return 0;
}

extern console_t console;
__attribute__((aligned(4096))) int _kernel_entry(bootinfo_t* init_data) {
    init(init_data);
    
    // for (int i = 0; i < 10; i++)
    //     kprintf("test %% %d %d %d"EOL, ((bootinfo_t*)init_data)->framebuffer.height, ((bootinfo_t*)init_data)->framebuffer.len_scanline, ((bootinfo_t*)init_data)->framebuffer.size);

    // for (int i = 0; i < init_data->map.size; ++i) {
    //     memory_descriptor_t *current = (memory_descriptor_t*)((uint8_t*)init_data->map.map + i * sizeof(memory_descriptor_t));
    //     kprintf("Memory segment no. [%d] type: %2u || phys start: %16llx || virt start: %16llx || npages: %lld || attributes: %2lld"EOL, i, current->type, current->phys_start, current->virt_start, current->npages, current->attribute);
    // }

    // tty_show_prompt();
    // for (int i = 0; i < 2000; ++i) {
    //     void *ptr = mmap_allocate_pages(1);
    //     kprintf("%16p"EOL, ptr);
    // }
    //
    // memory_descriptor_t *current = init_data->map.map + 1;
    // kprintf("Memory segment no. [%d] type: %s || phys start: %16llx || virt start: %16llx || npages: %lld || attributes: %2lld"EOL, 0, EFI_MEMORY_TYPE_STRING[current->type], current->phys_start, current->virt_start, current->npages, current->attribute);

    while (1) {
        __asm__("hlt");
    }
    
    return 0;
}
