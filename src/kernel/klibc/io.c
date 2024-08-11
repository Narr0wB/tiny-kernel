
#include <klibc/io.h>

int kprintf(const char *fmt) {
    while (*fmt) {
        write_char(*fmt++);
    }

    return 0;
}

