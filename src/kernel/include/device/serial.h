
#ifndef SERIAL_H
#define SERIAL_H

#include <common.h>
#include <util/lib.h>
#include <tty/tty.h>

int init_serial();

int serial_received();
int serial_read();

int serial_writeable();
void serial_write(char c);

static inline int serial_putc(char c) {
    serial_write(c);
    if (c == '\n') {
        serial_write('\r');
    }

    return c;
}
static inline int serial_puts(const char *string) {
    int count = 0;
    
    while (*string) {
        serial_write(*string);
        string++;
        count++;
    }
    
    return count;
}

#endif // SERIAL_H
