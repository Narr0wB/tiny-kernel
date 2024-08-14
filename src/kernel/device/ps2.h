
#ifndef PS2_H
#define PS2_H

#include <sys/types.h>
#include <klibc/io.h>

#define PS2_CTRL_DATA_PORT      0x60
#define PS2_CTRL_STATUS_PORT    0x64
#define PS2_CTRL_COMMAND_PORT   0x64

static inline bool ps2_output_buffer_status() {
    uint8_t status = inb(PS2_CTRL_STATUS_PORT);
    return status & (0x01 << 0);
}

static inline bool ps2_input_buffer_status() {
    uint8_t status = inb(PS2_CTRL_STATUS_PORT);
    return status & (0x01 << 1);
}

static inline void ps2_issue_command_controller(uint8_t command, uint8_t data_byte) {
    while (ps2_input_buffer_status()) {
        __asm__("nop");
    }
    
    outb(PS2_CTRL_COMMAND_PORT, command);
    io_wait();
}

static inline uint8_t ps2_read_response() {
    while (!ps2_output_buffer_status()) {
        __asm__("nop");
    }

    return inb(PS2_CTRL_DATA_PORT);
}

static inline void ps2_issue_command_port1(uint8_t command, uint8_t data_byte) {
     
}

#endif // PS2_h
