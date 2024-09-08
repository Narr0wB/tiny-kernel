
#include <util/io.h>

#define PRINTF_STATE_NORMAL        0
#define PRINTF_STATE_LENGTH        1
#define PRINTF_STATE_SPECIFIER     2
#define PRINTF_STATE_WIDTH         3

#define PRINTF_LENGTH_DEFAULT      0
#define PRINTF_LENGTH_LONG_LONG    1
#define PRINTF_LENGTH_HALF         2
#define PRINTF_LENGTH_HALF_HALF    3

const char hex_chars[] = "0123456789abcdef";
int print_number(putc_fn_t putc, uint8_t width, int64_t n, uint8_t sign, uint8_t radix) {
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
        putc(buffer[pos]);
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

int vsprintf(putc_fn_t pc, puts_fn_t ps, const char *fmt, va_list args) {
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
    va_list args_serial;
    va_list args_tty;
    va_start(args_serial, fmt);
    va_copy(args_tty, args_serial);
    
    int ret = vsprintf(serial_putc, serial_puts, fmt, args_serial);
    // if (IS_INITIALIZED(TTY_MODULE))
    {
        ret = vsprintf(tty_putc, tty_puts, fmt, args_tty);
    }

    va_end(args_tty);
    va_end(args_serial);
    return ret;
}

