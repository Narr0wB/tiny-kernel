
#include <video/video.h>

framebuffer_t *framebuffer;

void init_video(framebuffer_t *fb) {
    framebuffer = fb;

    uint32_t clear_color = 0;
    for (size_t i = 0; i < framebuffer->len_scanline * framebuffer->height; ++i) {
        ((uint32_t*)framebuffer->base_addr)[i] = clear_color;
    }
}

framebuffer_t *get_current_framebuffer() {
    return framebuffer;
}
