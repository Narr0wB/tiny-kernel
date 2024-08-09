
#include <video/video.h>
#include <video/font.h>

framebuffer_t *framebuffer;
psf_font_t font;

#define FONT_WIDTH 8

console_t console;

int init_video(framebuffer_t *fb) {
    framebuffer = fb;

    uint32_t clear_color = 0;
    for (size_t i = 0; i < framebuffer->len_scanline * framebuffer->height; ++i) {
        ((uint32_t*)framebuffer->base_addr)[i] = clear_color;
    }

    // Initialize the PSF font that is embedded in the kernel
    init_psf((void*)psf); // from font.h
    
    console.cell_size = FONT_WIDTH * font.header.character_size;
    console.padding_x = 2;
    console.padding_y = 1;
    console.cols = (framebuffer->len_scanline - 2 * (console.padding_x * FONT_WIDTH)) / FONT_WIDTH;
    console.rows = (framebuffer->len_scanline - 2 * (console.padding_y * font.header.character_size)) / font.header.character_size;

    return 0; 
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

int draw_char(char c, uint32_t x, uint32_t y) {
    if (c == '\n' || c == '\r') return 0;

    size_t offset = c * font.header.character_size;
    uint8_t *glyph_data = (uint8_t*)font.glyph_start + offset;
    uint32_t color = 0xffffffff;

    for (uint32_t i = y; i < (y + font.header.character_size) && i < framebuffer->height; ++i) {
        uint8_t glyph_row = glyph_data[i-y];

        for (uint32_t j = x; j < (x + FONT_WIDTH) && j < framebuffer->len_scanline; ++j) {
            size_t fb_index = i * framebuffer->len_scanline + j;
            
            ((uint32_t*)framebuffer->base_addr)[fb_index] = 0;
            if (get_bit(glyph_row,  (FONT_WIDTH - j + x))) {
                ((uint32_t*)framebuffer->base_addr)[fb_index] = color;
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

    if (console.current_cell == console.rows * console.cols)
        console.current_cell = 0;
    
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
