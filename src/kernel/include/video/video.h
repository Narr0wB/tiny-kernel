
#ifndef VIDEO_H
#define VIDEO_H

#include <common.h>
#include <boot/boot.h>
#include <util/lib.h>

#define CLEAR_COLOR 0x00000000

typedef struct {
    uint32_t len_scanline;
    uint32_t height;
    size_t size;
} framebuffer_info_t;

void init_video(framebuffer_t *fb);

framebuffer_t *vga_get_current_framebuffer();
void vga_set_pixel(uint32_t x, uint32_t y, uint32_t color);
void vga_set_pixel_index(size_t index, uint32_t color);
uint32_t vga_get_pixel(uint32_t x, uint32_t y);
uint32_t vga_get_pixel_index(size_t index);
void vga_clear_framebuffer(uint32_t clear_color);
framebuffer_info_t vga_get_current_framebuffer_info();

#endif // VIDEO_H
