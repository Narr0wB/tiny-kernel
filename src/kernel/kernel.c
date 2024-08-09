
#include <boot/boot.h>
#include <video/video.h>
#include <libc/stdio.h>

int init(bootinfo_t* init_data) {
    init_video(&init_data->framebuffer);

    printf("hello word! (type shii)\n");

    return 0;
}

int _kernel_entry(void* init_data) {
    return init((bootinfo_t*)init_data); 
}
