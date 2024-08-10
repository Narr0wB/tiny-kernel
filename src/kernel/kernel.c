
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
    init_mm();
    printf("hello word! (type shii)\n");
    
    return 0;
}
