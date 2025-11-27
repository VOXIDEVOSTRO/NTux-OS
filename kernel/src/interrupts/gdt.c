#include "gdt.h"
#include <stddef.h>
#include <stdint.h>  

#define GDT_ENTRIES 6  // Total number of GDT entries

// ---------------------------
// NTux-OS: Global Descriptor Table (GDT) structure
// ---------------------------
static struct gdt_entry gdt[GDT_ENTRIES];  // Array to hold all GDT entries
static struct gdt_ptr   gdt_descriptor;    // GDTR pointer structure for lgdt instruction

// ---------------------------
// NTux-OS: Helper to set a single GDT entry
//
// index       - position in GDT array
// base        - base address of the segment
// limit       - size of the segment
// access      - access flags (present, ring, code/data type)
// granularity - granularity flags and high bits of limit
// ---------------------------
static void gdt_set_entry(int index, uint32_t base, uint32_t limit, uint8_t access, uint8_t granularity) {
    gdt[index].limit_low    = (limit & 0xFFFF);           // Lower 16 bits of limit
    gdt[index].base_low     = (base & 0xFFFF);            // Lower 16 bits of base address
    gdt[index].base_middle  = (base >> 16) & 0xFF;        // Next 8 bits of base
    gdt[index].access       = access;                     // Access flags (type, privilege, present)
    gdt[index].granularity  = ((limit >> 16) & 0x0F)     // High 4 bits of limit
                             | (granularity & 0xF0);    // Upper granularity flags (G, D/B, L)
    gdt[index].base_high    = (base >> 24) & 0xFF;        // Highest 8 bits of base
}

// ---------------------------
// NTux-OS: Initialize the GDT
//
// This function:
// 1. Sets the GDTR structure
// 2. Creates a null segment as first entry (mandatory)
// 3. Loads the GDT into the CPU using assembly function gdt_load
// ---------------------------
void gdt_init(void) {
    // Set GDTR
    gdt_descriptor.limit = sizeof(struct gdt_entry) * GDT_ENTRIES - 1;
    gdt_descriptor.base  = (uint64_t)&gdt;

    // Null descriptor (mandatory)
    gdt_set_entry(0, 0, 0, 0, 0);
    // Kernel code segment: base=0, limit=4GB, executable, readable
    gdt_set_entry(1, 0, 0xFFFFFFFF, 0x9A, 0xA0);
    // Kernel data segment: base=0, limit=4GB, writable
    gdt_set_entry(2, 0, 0xFFFFFFFF, 0x92, 0xA0);

    // Load GDT
    gdt_load(&gdt_descriptor);
}