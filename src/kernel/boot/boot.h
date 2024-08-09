
#ifndef BOOT_H
#define BOOT_H 

#define FB_OFFSET 0

#include <video/video.h>

typedef struct bootinfo {
    framebuffer_t framebuffer;
} bootinfo_t;

#endif // BOOT_H

