
#include <video/video.h>

framebuffer_t *framebuffer;

void init_video(framebuffer_t *fb) {
    framebuffer = fb;

    uint32_t clear_color = 0;
    for (size_t i = 0; i < framebuffer->len_scanline * framebuffer->height; ++i) {
        ((uint32_t*)framebuffer->base_addr)[i] = clear_color;
    }
}

framebuffer_t *vga_get_current_framebuffer() {
    return framebuffer;
}

void vga_set_pixel(uint32_t x, uint32_t y, uint32_t color) {
    if (x > framebuffer->len_scanline || y > framebuffer->height) {
        return;
    }

    ((uint32_t*)framebuffer->base_addr)[y * framebuffer->len_scanline + x] = color;
}

void vga_set_pixel_index(size_t index, uint32_t color) {
    if (index > framebuffer->size/sizeof(uint32_t)) {
        return;
    }

    ((uint32_t*)framebuffer->base_addr)[index] = color;
}

uint32_t vga_get_pixel(uint32_t x, uint32_t y) {
    if (x > framebuffer->len_scanline || y > framebuffer->height) {
        return 0;
    }

    return ((uint32_t*)framebuffer->base_addr)[y * framebuffer->len_scanline + x];
}

uint32_t vga_get_pixel_index(size_t index) {
    if (index > framebuffer->size/sizeof(uint32_t)) {
        return 0;
    }

    return ((uint32_t*)framebuffer->base_addr)[index];
}

void vga_clear_framebuffer(uint32_t clear_color) {
    for (size_t i = 0; i < framebuffer->len_scanline; ++i) {
        for (size_t j = 0; j < framebuffer->height; ++j) {
            ((uint32_t*)framebuffer->base_addr)[j * framebuffer->len_scanline + i] = clear_color;
        }
    }
}

framebuffer_info_t vga_get_current_framebuffer_info() {
    return (framebuffer_info_t){framebuffer->len_scanline, framebuffer->height, framebuffer->size};
}
