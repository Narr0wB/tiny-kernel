
#include <klibc/string.h>

int is_digit(char c) {
    if ('0' >= c && c <= '9') return 1;
    return 0;
}

int digit(char c) {
    if (!is_digit(c)) return 0;

    return (c - '0');
}
