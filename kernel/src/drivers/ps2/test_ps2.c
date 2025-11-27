#include "test_ps2.h"
#include <kernel_lib/io.h>


// Initialization
void ps2_init() {
    // Disable both PS/2 ports
    outb(PS2_COMMAND_PORT, PS2_COMMAND_DISABLE_PORT1);
    outb(PS2_COMMAND_PORT, PS2_COMMAND_DISABLE_PORT2);

    // Flush output buffer
    while (inb(PS2_STATUS_PORT) & PS2_STATUS_OUTPUT_FULL)
        inb(PS2_DATA_PORT);

    // Read controller configuration byte
    outb(PS2_COMMAND_PORT, PS2_COMMAND_READ_CONFIG);
    uint8_t config = inb(PS2_DATA_PORT);

    // Enable PS/2 ports
    config |= 1;    // Enable first port
    config &= ~0x10; // Enable IRQ12 for first port
    outb(PS2_COMMAND_PORT, PS2_COMMAND_WRITE_CONFIG);
    outb(PS2_DATA_PORT, config);

    outb(PS2_COMMAND_PORT, PS2_COMMAND_ENABLE_PORT1); // Enable first port
}

// Read keyboard data
uint8_t ps2_read_data() {
    while (!(inb(PS2_STATUS_PORT) & PS2_STATUS_OUTPUT_FULL))
        ; // Wait until output buffer is full
    return inb(PS2_DATA_PORT);
}

uint16_t sct1[] = {
    0, 0x0100, 49, 50, // unprintable = escape
    51, 52, 53, 54,
    55, 56, 57, 48,
    45, 61, 0x0200, 9,  // unprintable = backspace
    113, 119, 101, 114,
    116, 121, 117, 105,
    111, 112, 91, 93,
    0x0300, 0x0400, 97, 115, // unprintable = enter, control
    100, 102, 103, 104,
    106, 107, 108, 59,
    39, 96, 0x0500, 92, // unprintable = shift
    122, 120, 99, 118,
    98, 110, 109, 44,
    46, 47, 0x0500, 42, // unprintable = shift
    0x0600, 32, 0x0700, 0x0800, // unprintable = alt, CapsLock, f1
    0x0900, 0x0A00, 0x0B00, 0x0C00, // unprintable = f2, f3, f4, f5
    0x0D00, 0x0E00, 0x0F00, 0x1000, // unprintable = f6, f7, f8, f9
    0x1100, 0x1400, 0x1500, 55, // unprintable = f10, NumLock, ScrollLock  (rule broken here to make Function keys more logical)
    56, 57, 45, 52,
    53, 54, 43, 49,
    50, 51, 48, 46,
    0, 0, 0, 0x1200, // unprintable = f11
    0x1300           // unprintable = f12
};

uint16_t detect_special_key(uint8_t scancode) {
    if (scancode >= sizeof(sct1)/sizeof(sct1[0])) return 0;
    uint16_t key = sct1[scancode];

    if (key & 0xFF00) {
        return key;    }

    return 0;
}