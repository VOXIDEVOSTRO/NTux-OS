#pragma once

#include <stdint.h>
#include <stddef.h>


// I/O Ports
#define PS2_DATA_PORT   0x60    // PS/2 Data port
#define PS2_STATUS_PORT 0x64    // PS/2 Status port
#define PS2_COMMAND_PORT 0x64   // PS/2 Command port

// Status Register Bits
#define PS2_STATUS_OUTPUT_FULL 0x01
#define PS2_STATUS_INPUT_FULL  0x02

// Commands
#define PS2_COMMAND_READ_CONFIG 0x20
#define PS2_COMMAND_WRITE_CONFIG 0x60
#define PS2_COMMAND_ENABLE_PORT1 0xAE
#define PS2_COMMAND_DISABLE_PORT1 0xAD
#define PS2_COMMAND_ENABLE_PORT2 0xA8
#define PS2_COMMAND_DISABLE_PORT2 0xA7
#define PS2_COMMAND_READ_OUTPUT 0xD0
#define PS2_COMMAND_WRITE_OUTPUT 0xD1



// Initialization
void ps2_init();

// Read keyboard data
uint8_t ps2_read_data();
uint16_t detect_special_key(uint8_t scancode);


//array containing ascii values(if unprintable -> starting at 0x0100 increasing in order of table) indexable by scancodes from table 1

extern uint16_t sct1[];