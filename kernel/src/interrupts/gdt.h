#ifndef GDT_H
#define GDT_H  

#include <stdint.h>

// ---------------------------
// NTux-OS: Single GDT entry structure
// ---------------------------
// Each entry in the Global Descriptor Table (GDT) describes a memory segment.
// The __attribute__((packed)) ensures the compiler does not add padding between fields.
struct gdt_entry {
    uint16_t limit_low;     // Lower 16 bits of the segment limit
    uint16_t base_low;      // Lower 16 bits of the segment base address
    uint8_t  base_middle;   // Next 8 bits of the base address
    uint8_t  access;        // Access flags: present, privilege level, code/data type
    uint8_t  granularity;   // Granularity flags and high 4 bits of segment limit
    uint8_t  base_high;     // Highest 8 bits of the base address
} __attribute__((packed));

// ---------------------------
// NTux-OS: GDTR structure
// ---------------------------
// This structure is used with the LGDT instruction to load the GDT into the CPU.
// It contains the size of the GDT and a pointer to its base in memory.
struct gdt_ptr {
    uint16_t limit;         // Size of GDT in bytes - 1
    uint64_t base;          // Address of the first GDT entry
} __attribute__((packed));

// ---------------------------
// NTux-OS: GDT entry indices
// ---------------------------
// These defines give a human-readable name to each GDT entry index.
#define GDT_NULL      0      // Null segment (mandatory first entry)

// ---------------------------
// NTux-OS: Function prototypes
// ---------------------------
void gdt_init(void);                    // Initialize the GDT and load it
extern void gdt_load(struct gdt_ptr* gdt_descriptor);  // Assembly function to load GDTR

#endif