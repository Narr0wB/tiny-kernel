
#ifndef TTY_H
#define TTY_H

#include <common.h>
#include <video/video.h>

#define PSF_MAGIC 0x3604
#define FONT_WIDTH 8

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

void init_tty();
int init_psf(void *data);

void tty_offset_line(uint8_t lines);  
int draw_char(char c, uint32_t x, uint32_t y);
int write_char(char c);

#endif // TTY_H
