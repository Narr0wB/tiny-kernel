
#ifndef VIDEO_H
#define VIDEO_H

#include <common.h>
#include <boot/boot.h>
#include <klibc/lib.h>

#define PSF_MAGIC 0x3604

typedef struct psf_header {
    uint16_t magic;
    uint8_t font_mode;
    uint8_t character_size;
} psf_header_t;

typedef struct psf_font {
    psf_header_t header;
    uint16_t glyph_count;
    void *glyph_start;
} psf_font_t;

typedef struct console {
    uint8_t  cell_size;
    uint32_t rows;
    uint32_t cols;
    uint8_t padding_x;
    uint8_t padding_y;

    uint32_t current_cell;
} console_t;

int init_video(framebuffer_t *fb);
int init_psf(void *data);

int draw_char(char c, uint32_t x, uint32_t y);
int write_char(char c);

#endif // VIDEO_H
