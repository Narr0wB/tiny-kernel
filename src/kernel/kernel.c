
#include <boot/boot.h>
#include <video/video.h>
#include <libc/stdio.h>
#include <memory/memory.h>

int init(bootinfo_t* init_data) {
    init_video(&init_data->framebuffer);
    return 0;
}

int _kernel_entry(void* init_data) {
    init((bootinfo_t*)init_data);
    printf("hello world!\n");
    init_mm();

    while (1) {
        __asm__("nop");
    }
    
    return 0;
}
