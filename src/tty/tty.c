
#include <tty/tty.h>
#include <tty/font.h>
#include <util/modules.h>

int TTY_MODULE = 0;

static framebuffer_info_t fb_info;
static console_t console;

char stdin[4096] = {0};
uint16_t stdin_pos = 0;

void init_tty() {
    init_psf((void*)psf);

    fb_info = vga_get_current_framebuffer_info();

    console.cell_size = FONT_WIDTH * console.font.header.character_size;
    console.padding_x = 0;
    console.padding_y = 0;
    console.cols = (fb_info.len_scanline - 2 * (console.padding_x * FONT_WIDTH)) / FONT_WIDTH;
    console.rows = (fb_info.height - 2 * (console.padding_y * console.font.header.character_size)) / console.font.header.character_size;
    
    INITIALIZED(TTY_MODULE);
}

int init_psf(void *data) {
    // Commence by parsing the PSF header (PSF version 1)
    console.font.header = *(psf_header_t*)data;

    console.font.glyph_count = 256;

    if (console.font.header.font_mode != 0) {
        console.font.glyph_count = 512;
    }
    
    const size_t psf_offset = sizeof(psf_header_t);
    console.font.glyph_start = (void*)((uint8_t*)data + psf_offset);

    return 0;
}


int draw_char(char c, uint32_t x, uint32_t y) {
    if (c == '\n' || c == '\r') return -1;

    size_t offset = c * console.font.header.character_size;
    uint8_t *glyph_data = (uint8_t*)console.font.glyph_start + offset;
    const uint32_t color = 0xffffffff;

    for (uint32_t i = y; i < (y + console.font.header.character_size) && i < fb_info.height; ++i) {
        uint8_t glyph_row = glyph_data[i-y];

        for (uint32_t j = x; j < (x + FONT_WIDTH) && j < fb_info.len_scanline; ++j) {
            
            vga_set_pixel(j, i, CLEAR_COLOR);
            if (get_bit(glyph_row, (FONT_WIDTH - j + x))) {
                vga_set_pixel(j, i, color);
            }
        } 
    }

    return 0;
}

int write_char(char c) {
    int special = 0;
    switch (c) {
        case '\n': {
            console.current_cell += console.cols;
            console.current_cell -= (console.current_cell % console.cols);
            special = 1;
            break;
        }

        case '\r': {
            console.current_cell -= (console.current_cell % console.cols);
            special = 1;
            break;
        }
    }

    if (console.current_cell == console.rows * console.cols) {
        console.current_cell = (console.rows - 1) * console.cols;
        tty_offset_line(1);
    }

    uint32_t col = console.current_cell % console.cols;
    uint32_t row = console.current_cell / console.cols;

    uint32_t x = console.padding_x * FONT_WIDTH + col * FONT_WIDTH;
    uint32_t y = console.padding_y * console.font.header.character_size + row * console.font.header.character_size;

    if (!special) {
        draw_char(c, x, y);
        console.current_cell++;
    }

    return 0;
}

void tty_offset_line(uint32_t lines) {
    uint32_t height_offset = console.font.header.character_size * fb_info.len_scanline;

    for (uint32_t i = 0; i < lines; ++i) {
        for (size_t j = 0; j < fb_info.len_scanline * fb_info.height; ++j) {
            if (((fb_info.len_scanline * fb_info.height) - j) < height_offset) {
                vga_set_pixel_index(j, CLEAR_COLOR);
            } 

            vga_set_pixel_index(j, vga_get_pixel_index(j + height_offset));
        }
    }
}

void tty_set_cursor(cursor_t cursor) {
    console.current_cell = cursor.y * console.cols + cursor.x;
}

cursor_t tty_get_cursor(uint32_t cell) {
    cursor_t c = { console.current_cell % console.cols, console.current_cell / console.cols}; 
    return c; 
}

void tty_clear(uint32_t clear_color) {
    vga_clear_framebuffer(clear_color); 
}

void tty_show_prompt() {
    cursor_t cursor = tty_get_cursor(console.current_cell);
    if (cursor.y == (console.rows - 1)) {
        tty_offset_line(1);
        cursor = tty_get_cursor(console.current_cell);
    }

    tty_set_cursor((cursor_t){0, cursor.y});
    write_char('$');
    write_char(' ');

    for (uint16_t i = 0; i < stdin_pos; ++i) {
        write_char(stdin[i]);
    }
}

int tty_putc(char c) {
    write_char(c);

    return c;
}

int tty_puts(const char *str) {
    int counter;

    while (*str) {
        tty_putc(*str++);
        counter++;
    }

    return counter; 
}
