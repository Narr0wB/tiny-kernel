
#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <device/ps2.h>
#include <int/int.h>
#include <memory/memory.h>

typedef enum { 
    VK_ESCAPE = 0x01, 
    VK_1, 
    VK_2, 
    VK_3, 
    VK_4, 
    VK_5, 
    VK_6, 
    VK_7, 
    VK_8, 
    VK_9, 
    VK_0, 
    VK_MINUS, 
    VK_EQUALS, 
    VK_BACKSPACE,
    VK_TAB, 
    VK_Q, 
    VK_W, 
    VK_E, 
    VK_R, 
    VK_T, 
    VK_Y, 
    VK_U, 
    VK_I, 
    VK_O, 
    VK_P, 
    VK_LEFTBRACE, 
    VK_RIGHTBRACE, 
    VK_ENTER,
    VK_LEFTCONTROL, 
    VK_A, 
    VK_S, 
    VK_D, 
    VK_F, 
    VK_G, 
    VK_H, 
    VK_J, 
    VK_K, 
    VK_L, 
    VK_SEMICOLON, 
    VK_APOSTROPHE, 
    VK_BACKTICK,
    VK_LEFTSHITFT, 
    VK_BACKSLASH, 
    VK_Z, 
    VK_X, 
    VK_C, 
    VK_V, 
    VK_B, 
    VK_N, 
    VK_M, 
    VK_COMMA, 
    VK_DOT, 
    VK_SLASH,
    VK_RIGHTSHIFT,
    VK_KPASTERISK,
    VK_LEFTALT,
    VK_SPACE,
    VK_CAPSLOCK, 
    VK_F1, 
    VK_F2, 
    VK_F3,
    VK_F4,
    VK_F5, 
    VK_F6, 
    VK_F7, 
    VK_F8, 
    VK_F9, 
    VK_F10, 
    VK_NUMLOCK, 
    VK_SCROLLLOCK,
    VK_KP7,
    VK_KP8,
    VK_KP9,
    VK_KPMINUS,
    VK_KP4, 
    VK_KP5,
    VK_KP6,
    VK_KPPLUS,
    VK_KP1,
    VK_KP2,
    VK_KP3,
    VK_KP0,
    VK_KPDOT
} VirtualKey;

typedef enum {
    SHIFT = 0b10000000,
    CTRL  = 0b01000000,
    ALT   = 0b00100000,
} Modifiers;

typedef enum {
    PRESSED  = 0b00000001,
    RELEASED = 0b00000000 
} State;

typedef struct {
    uint8_t modifiers;
    uint8_t key;
} keycode_t;

void init_keyboard();

static char scan_to_char[] = {
    0,
    0,
    '1',
    '2',
    '3',
    '4',
    '5',
    '6',
    '7',
    '8',
    '9',
    '0',
    '-',
    '=',
    0,
    '\t',
    'q',
    'w',
    'e',
};

#endif // KEYBOARD_H
