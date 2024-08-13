
#include <klibc/io.h>

#define PRINTF_STATE_NORMAL        0
#define PRINTF_STATE_LENGTH        1
#define PRINTF_STATE_SPECIFIER     2

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
int print_number(void *putc, int64_t n, uint8_t sign, uint8_t radix) {
    char buffer[32];
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
    while (number > radix) {
        buffer[pos++] = hex_chars[(number % radix)];
        number /= radix;
    } 
    buffer[pos++] = hex_chars[(number % radix)];

    counter = pos;

    if (sign && n_sign == -1) {
        buffer[pos++] = '-';
    }

    while (pos-- > 0) {
        pc(buffer[pos]);
    }

    return counter;
}

int vsprintf(void *_pc, void *_ps, const char *fmt, va_list args) {
    int (*pc)(int) = ((int(*)(int))_pc); 
    int (*ps)(const char *) = ((int(*)(const char *))_ps); 

    uint8_t *argp = 0;

    uint8_t state = 0; 
    uint8_t len = 0;
    while (*fmt) {
        if (*fmt == 0) break;
        if (state == PRINTF_STATE_NORMAL && *fmt == '%') {
            state = PRINTF_STATE_LENGTH;
            fmt++;
        }
        else if (state == PRINTF_STATE_LENGTH && *fmt == 'l') {
            len = PRINTF_LENGTH_LONG_LONG; 
            fmt++;
        }
        else if (state == PRINTF_STATE_LENGTH && *fmt == 'h') {
            if (len == PRINTF_LENGTH_HALF) len = PRINTF_LENGTH_HALF_HALF;
            else len = PRINTF_LENGTH_HALF;
            fmt++;
        }
        else if (state == PRINTF_STATE_LENGTH && (*fmt != 'l' || *fmt != 'h')) {
            state = PRINTF_STATE_SPECIFIER;
            continue;
        }
        else if (state == PRINTF_STATE_SPECIFIER) {
            switch (*fmt) {
                case 's': {
                    ps(va_arg(args, const char*));
                    break;
                }

                case 'c': {
                    pc(va_arg(args, int)); 
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
                    print_number(pc, n, 1, 10);  
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
                    print_number(pc, n, 0, 10);
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
                    print_number(pc, n, 0, 16);
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
                    print_number(pc, n, 0, 8);
                    break;
                }

                case '%': {
                    pc('%');
                    break;
                }
                
                default: break;
            }

            fmt++;
            state = PRINTF_STATE_NORMAL;
        }
        else {
            putchar(*fmt++);
        }
    }
    
    return 0;
}


int kprintf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    
    vsprintf((void*)&putchar, (void*)&puts, fmt, args);

    va_end(args);
    return 0;
}

