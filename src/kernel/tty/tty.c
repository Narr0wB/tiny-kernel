
#include <tty/tty.h>
#include <tty/font.h>

psf_font_t font;
framebuffer_t *tty_framebuffer;
console_t console;

void init_tty() {
    tty_framebuffer = get_current_framebuffer();

    init_psf((void*)psf);

    console.cell_size = FONT_WIDTH * font.header.character_size;
    console.padding_x = 0;
    console.padding_y = 0;
    console.cols = (tty_framebuffer->len_scanline - 2 * (console.padding_x * FONT_WIDTH)) / FONT_WIDTH;
    console.rows = (tty_framebuffer->height - 2 * (console.padding_y * font.header.character_size)) / font.header.character_size;
}

int init_psf(void *data) {
    // Commence by parsing the PSF header (PSF version 1)
    font.header = *(psf_header_t*)data;

    font.glyph_count = 256;

    if (font.header.font_mode != 0) {
        font.glyph_count = 512;
    }
    
    const size_t psf_offset = sizeof(psf_header_t);
    font.glyph_start = (void*)((uint8_t*)data + psf_offset);

    return 0;
}

void tty_offset_line(uint8_t lines) {
    uint32_t height_offset = font.header.character_size * tty_framebuffer->len_scanline;

    for (uint8_t i = 0; i < lines; ++i) {
        for (size_t j = 0; j < tty_framebuffer->len_scanline * tty_framebuffer->height; ++j) {
            if ((tty_framebuffer->size - j) < height_offset) {
                ((uint32_t*)tty_framebuffer->base_addr)[j] = CLEAR_COLOR;
            } 

            ((uint32_t*)tty_framebuffer->base_addr)[j] = ((uint32_t*)tty_framebuffer->base_addr)[j + height_offset];
        }
    }
}

int draw_char(char c, uint32_t x, uint32_t y) {
    if (c == '\n' || c == '\r') return 0;

    size_t offset = c * font.header.character_size;
    uint8_t *glyph_data = (uint8_t*)font.glyph_start + offset;
    uint32_t color = 0xffffffff;

    for (uint32_t i = y; i < (y + font.header.character_size) && i < tty_framebuffer->height; ++i) {
        uint8_t glyph_row = glyph_data[i-y];

        for (uint32_t j = x; j < (x + FONT_WIDTH) && j < tty_framebuffer->len_scanline; ++j) {
            size_t fb_index = i * tty_framebuffer->len_scanline + j;
            
            ((uint32_t*)tty_framebuffer->base_addr)[fb_index] = 0;
            if (get_bit(glyph_row,  (FONT_WIDTH - j + x))) {
                ((uint32_t*)tty_framebuffer->base_addr)[fb_index] = color;
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
    uint32_t y = console.padding_y * font.header.character_size + row * font.header.character_size;

    if (!special) {
        draw_char(c, x, y);
        console.current_cell++;
    }

    return 0;
}
