
#ifndef PS2_H
#define PS2_H

#include <sys/types.h>
#include <util/io.h>

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

static inline void ps2_issue_command(uint8_t command) {
    while (ps2_input_buffer_status()) {
        __asm__("nop");
    }
    
    outb(PS2_CTRL_COMMAND_PORT, command);
    io_wait();
}

static inline void ps2_write_data(uint8_t data) {
    while (ps2_input_buffer_status()) {
        __asm__("nop");
    }
    
    outb(PS2_CTRL_DATA_PORT, data);
    io_wait();
}

static inline uint8_t ps2_read_data() {
    while (!ps2_output_buffer_status()) {
        __asm__("nop");
    }

    return inb(PS2_CTRL_DATA_PORT);
}

#endif // PS2_h
