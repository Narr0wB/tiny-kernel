
#include <libc/stdio.h>

int printf(const char *fmt) {
    while (*fmt) {
        write_char(*fmt++);
    }

    return 0;
}
