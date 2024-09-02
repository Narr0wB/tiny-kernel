
#ifndef TTY_H
#define TTY_H

#include <common.h>
#include <stdarg.h>
#include <video/video.h>
#include <util/string.h>

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
    uint8_t cell_size;
    uint32_t rows;
    uint32_t cols;
    uint8_t padding_x;
    uint8_t padding_y;

    uint32_t current_cell;
    psf_font_t font;
} console_t;

typedef struct cursor {
    uint32_t x;
    uint32_t y;
} cursor_t;

void init_tty();
int init_psf(void *data);

int draw_char(char c, uint32_t x, uint32_t y);
int write_char(char c);

void tty_offset_line(uint32_t lines);  
void tty_set_cursor(cursor_t cursor);
void tty_clear(uint32_t clear_color);
void tty_show_prompt();

#define EOL "\n"

int putchar(int c);
int puts(const char *str);
int kprintf(const char *fmt, ...);

int vsprintf(void *pc, void *ps, const char *fmt, va_list args);
int sprintf(char *buffer, const char *fmt, ...);

#endif // TTY_H
