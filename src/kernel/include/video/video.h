
#ifndef VIDEO_H
#define VIDEO_H

#include <common.h>
#include <boot/boot.h>
#include <klibc/lib.h>

#define CLEAR_COLOR 0x00000000

void init_video(framebuffer_t *fb);
framebuffer_t *get_current_framebuffer();

#endif // VIDEO_H
