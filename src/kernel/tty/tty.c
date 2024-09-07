
#include <tty/tty.h>
#include <int/int.h>
#include <memory/memory.h>
#include <tty/font.h>

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

#define PRINTF_STATE_NORMAL        0
#define PRINTF_STATE_LENGTH        1
#define PRINTF_STATE_SPECIFIER     2
#define PRINTF_STATE_WIDTH         3

#define PRINTF_LENGTH_DEFAULT      0
#define PRINTF_LENGTH_LONG_LONG    1
#define PRINTF_LENGTH_HALF         2
#define PRINTF_LENGTH_HALF_HALF    3

int putchar(int c) {
    write_char(c);

    return c;
}

int puts(const char *str) {
    int counter;

    while (*str) {
        putchar(*str++);
        counter++;
    }

    return counter; 
}

const char hex_chars[] = "0123456789abcdef";
int print_number(void *putc, uint8_t width, int64_t n, uint8_t sign, uint8_t radix) {
    char buffer[32] = {[0 ... 31] = '0'};
    uint64_t number;
    int8_t n_sign = 1;
    int (*pc)(int) = ((int(*)(int))putc); 

    if (sign) {
        if (n < 0) {
            n = -n;
            n_sign = -1;
        } 
        number = n;
    }
    else {
        number = (uint64_t)n;
    }
   
    int8_t pos = 0, counter = 0;
    while (number >= radix) {
        buffer[pos++] = hex_chars[(number % radix)];
        number /= radix;
    } 
    buffer[pos++] = hex_chars[(number % radix)];

    if (width > 0) {
        pos += (width - pos);
    }

    counter = pos;

    if (sign && n_sign == -1) {
        buffer[pos++] = '-';
    }

    while (pos-- > 0) {
        pc(buffer[pos]);
    }

    return counter;
}

int print_number_buffer(char **buf, uint8_t width, int64_t n, uint8_t sign, uint8_t radix) {
    char buffer[32] = {[0 ... 31] = '0'};
    uint64_t number;
    int8_t n_sign = 1;

    if (sign) {
        if (n < 0) {
            n = -n;
            n_sign = -1;
        } 
        number = n;
    }
    else {
        number = (uint64_t)n;
    }
   
    int8_t pos = 0, counter = 0;
    while (number >= radix) {
        buffer[pos++] = hex_chars[(number % radix)];
        number /= radix;
    } 
    buffer[pos++] = hex_chars[(number % radix)];

    if (width > 0) {
        pos += (width - pos);
    }

    counter = pos;

    if (sign && n_sign == -1) {
        buffer[pos++] = '-';
    }

    while (pos-- > 0) {
        *(*buf)++ = (buffer[pos]);
    }

    return counter;
}

int vsprintf(void *_pc, void *_ps, const char *fmt, va_list args) {
    int (*pc)(int) = ((int(*)(int))_pc); 
    int (*ps)(const char *) = ((int(*)(const char *))_ps); 

    int written = 0;
    uint8_t state = PRINTF_STATE_NORMAL; 
    uint8_t width = 0;
    uint8_t len = 0;

    while (*fmt) {
        switch (state) {
            case PRINTF_STATE_NORMAL: {
                if (*fmt == '%') {
                    state = PRINTF_STATE_WIDTH;
                    fmt++;
                }
                else {
                    pc(*fmt++);
                    written++;
                }

                break;
            }
            case PRINTF_STATE_WIDTH: {
                if (is_digit(*fmt)) {
                    width *= 10;
                    width += digit(*fmt);
                    fmt++;
                }
                else {
                    state = PRINTF_STATE_LENGTH;
                }

                break;
            }
            case PRINTF_STATE_LENGTH: {
                if (*fmt == 'l') {
                    len = PRINTF_LENGTH_LONG_LONG; 
                    fmt++; 
                }
                else if (*fmt == 'h') {
                    if (len == PRINTF_LENGTH_HALF) {len = PRINTF_LENGTH_HALF_HALF;}
                    else len = PRINTF_LENGTH_HALF;
                    fmt++; 
                }
                else {
                    state = PRINTF_STATE_SPECIFIER;
                }

                break;
            }
            case PRINTF_STATE_SPECIFIER: {
                switch (*fmt) {
                    case 's': {
                        written += ps(va_arg(args, const char*));
                        break;
                    }

                    case 'c': {
                        pc(va_arg(args, int)); 
                        written++;
                        break;
                    }
                    
                    case 'i':
                    case 'd': {
                        int64_t n = 0;
                        switch (len) {
                            case PRINTF_LENGTH_HALF_HALF:
                                n = va_arg(args, int8_t);
                            break;
                            case PRINTF_LENGTH_HALF:
                                n = va_arg(args, int16_t);
                            break;
                            case PRINTF_LENGTH_DEFAULT:
                                n = va_arg(args, int32_t);
                            break;
                            case PRINTF_LENGTH_LONG_LONG:
                                n = va_arg(args, int64_t);
                            break;
                        }
                        written += print_number(pc, width, n, 1, 10);  
                        break;
                    }

                    case 'u': {
                        int64_t n = 0;
                        switch (len) {
                            case PRINTF_LENGTH_HALF_HALF:
                                n = va_arg(args, uint8_t);
                            break;
                            case PRINTF_LENGTH_HALF:
                                n = va_arg(args, uint16_t);
                            break;
                            case PRINTF_LENGTH_DEFAULT:
                                n = va_arg(args, uint32_t);
                            break;
                            case PRINTF_LENGTH_LONG_LONG:
                                n = va_arg(args, uint64_t);
                            break;
                        }
                        written += print_number(pc, width, n, 0, 10);
                        break;
                    }
                    
                    case 'p':
                    case 'X':
                    case 'x': {
                        int64_t n = 0;
                        switch (len) {
                            case PRINTF_LENGTH_HALF_HALF:
                                n = va_arg(args, uint8_t);
                            break;
                            case PRINTF_LENGTH_HALF:
                                n = va_arg(args, uint16_t);
                            break;
                            case PRINTF_LENGTH_DEFAULT:
                                n = va_arg(args, uint32_t);
                            break;
                            case PRINTF_LENGTH_LONG_LONG:
                                n = va_arg(args, uint64_t);
                            break;
                        }
                        written += print_number(pc, width, n, 0, 16);
                        break;
                    }

                    case 'o': {
                        int64_t n = 0;
                        switch (len) {
                            case PRINTF_LENGTH_HALF_HALF:
                                n = va_arg(args, uint8_t);
                            break;
                            case PRINTF_LENGTH_HALF:
                                n = va_arg(args, uint16_t);
                            break;
                            case PRINTF_LENGTH_DEFAULT:
                                n = va_arg(args, uint32_t);
                            break;
                            case PRINTF_LENGTH_LONG_LONG:
                                n = va_arg(args, uint64_t);
                            break;
                        }
                        written += print_number(pc, width, n, 0, 8);
                        break;
                    }

                    case '%': {
                        pc('%');
                        written++;
                        break;
                    }
                    
                    default: break;
                }

                fmt++;
                state = PRINTF_STATE_NORMAL;
                width = 0;
                break;
            }
        }
    }
    
    return written;
}

int sprintf(char *buffer, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    int written = 0;
    uint8_t state = PRINTF_STATE_NORMAL; 
    uint8_t width = 0;
    uint8_t len = 0;

    while (*fmt) {
        switch (state) {
            case PRINTF_STATE_NORMAL: {
                if (*fmt == '%') {
                    state = PRINTF_STATE_WIDTH;
                    fmt++;
                }
                else {
                    *buffer++ = (*fmt++);
                    written++;
                }

                break;
            }
            case PRINTF_STATE_WIDTH: {
                if (is_digit(*fmt)) {
                    width *= 10;
                    width += digit(*fmt);
                    fmt++;
                }
                else {
                    state = PRINTF_STATE_LENGTH;
                }

                break;
            }
            case PRINTF_STATE_LENGTH: {
                if (*fmt == 'l') {
                    len = PRINTF_LENGTH_LONG_LONG; 
                    fmt++; 
                }
                else if (*fmt == 'h') {
                    if (len == PRINTF_LENGTH_HALF) {len = PRINTF_LENGTH_HALF_HALF;}
                    else len = PRINTF_LENGTH_HALF;
                    fmt++; 
                }
                else {
                    state = PRINTF_STATE_SPECIFIER;
                }

                break;
            }
            case PRINTF_STATE_SPECIFIER: {
                switch (*fmt) {
                    case 's': {
                        const char *str = va_arg(args, const char*);
                        while (*str) {
                            *buffer++ = *str++;
                            written++;
                        }
                        break;
                    }

                    case 'c': {
                        *buffer++ = (va_arg(args, int)); 
                        written++;
                        break;
                    }
                    
                    case 'i':
                    case 'd': {
                        int64_t n = 0;
                        switch (len) {
                            case PRINTF_LENGTH_HALF_HALF:
                                n = va_arg(args, int8_t);
                            break;
                            case PRINTF_LENGTH_HALF:
                                n = va_arg(args, int16_t);
                            break;
                            case PRINTF_LENGTH_DEFAULT:
                                n = va_arg(args, int32_t);
                            break;
                            case PRINTF_LENGTH_LONG_LONG:
                                n = va_arg(args, int64_t);
                            break;
                        }
                        written += print_number_buffer(&buffer, width, n, 1, 10);  
                        break;
                    }

                    case 'u': {
                        int64_t n = 0;
                        switch (len) {
                            case PRINTF_LENGTH_HALF_HALF:
                                n = va_arg(args, uint8_t);
                            break;
                            case PRINTF_LENGTH_HALF:
                                n = va_arg(args, uint16_t);
                            break;
                            case PRINTF_LENGTH_DEFAULT:
                                n = va_arg(args, uint32_t);
                            break;
                            case PRINTF_LENGTH_LONG_LONG:
                                n = va_arg(args, uint64_t);
                            break;
                        }
                        written += print_number_buffer(&buffer, width, n, 0, 10);
                        break;
                    }
                    
                    case 'p':
                    case 'X':
                    case 'x': {
                        int64_t n = 0;
                        switch (len) {
                            case PRINTF_LENGTH_HALF_HALF:
                                n = va_arg(args, uint8_t);
                            break;
                            case PRINTF_LENGTH_HALF:
                                n = va_arg(args, uint16_t);
                            break;
                            case PRINTF_LENGTH_DEFAULT:
                                n = va_arg(args, uint32_t);
                            break;
                            case PRINTF_LENGTH_LONG_LONG:
                                n = va_arg(args, uint64_t);
                            break;
                        }
                        written += print_number_buffer(&buffer, width, n, 0, 16);
                        break;
                    }

                    case 'o': {
                        int64_t n = 0;
                        switch (len) {
                            case PRINTF_LENGTH_HALF_HALF:
                                n = va_arg(args, uint8_t);
                            break;
                            case PRINTF_LENGTH_HALF:
                                n = va_arg(args, uint16_t);
                            break;
                            case PRINTF_LENGTH_DEFAULT:
                                n = va_arg(args, uint32_t);
                            break;
                            case PRINTF_LENGTH_LONG_LONG:
                                n = va_arg(args, uint64_t);
                            break;
                        }
                        written += print_number_buffer(&buffer, width, n, 0, 8);
                        break;
                    }

                    case '%': {
                        *buffer++ = ('%');
                        written++;
                        break;
                    }
                    
                    default: break;
                }

                fmt++;
                state = PRINTF_STATE_NORMAL;
                break;
            }
        }
    }
   
    *buffer = 0;
    return written;

    va_end(args); 
}

int kprintf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    
    int ret = vsprintf((void*)&putchar, (void*)&puts, fmt, args);

    va_end(args);
    return ret;
}
