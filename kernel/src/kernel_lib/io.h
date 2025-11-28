#ifndef IO_H
#define IO_H

#include <stdint.h>

// Function prototypes for I/O operations

// Reads a byte (8-bit) from the specified I/O port
uint8_t inb(uint16_t port);

// Writes a byte (8-bit) to the specified I/O port
void outb(uint16_t port, uint8_t value);

// Reads a word (16-bit) from the specified I/O port
uint16_t inw(uint16_t port);

// Writes a word (16-bit) to the specified I/O port
void outw(uint16_t port, uint16_t value);

//for pci
uint32_t inl(uint16_t port);
void     outl(uint16_t port, uint32_t value);

#endif // IO_H
