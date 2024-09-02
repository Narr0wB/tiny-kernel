
#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <device/ps2.h>

typedef enum {
    SCAN_CODE_A_PRESSED
} SCAN_CODE_SET;

static char scan_to_char[] = {
    0,
    0,
    '1',
    '2',
    '3',
    '4'
};

void init_keyboard();

#endif // KEYBOARD_H
