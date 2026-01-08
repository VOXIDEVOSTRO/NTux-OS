#ifndef GDT_H
#define GDT_H

#include <stdint.h>

// ---------------------------
// NTux-OS: Single GDT entry structure
// ---------------------------
struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_middle;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
} __attribute__((packed));

// ---------------------------
// NTux-OS: GDTR structure
// ---------------------------
struct gdt_ptr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

// ---------------------------
// NTux-OS: 64-bit Task State Segment (TSS)
// ---------------------------
struct tss_entry {
    uint32_t reserved0;
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t reserved1;
    uint64_t ist[7];
    uint64_t reserved2;
    uint16_t reserved3;
    uint16_t iomap_base;
} __attribute__((packed));

// ---------------------------
// NTux-OS: GDT entry indices
// ---------------------------
#define GDT_NULL      0
#define GDT_CODE64    1
#define GDT_DATA64    2
#define GDT_TSS       3   // TSS uses index 3 and 4 (16 bytes = 2 entries)

// Total entries: 6 (0-5)
#define GDT_ENTRIES   6

// ---------------------------
// NTux-OS: Function prototypes
// ---------------------------
void gdt_init(void);
extern void gdt_load(struct gdt_ptr* gdt_descriptor);

#endif