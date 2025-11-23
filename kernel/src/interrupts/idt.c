#include "idt.h"
#include <interrupts/irq.h>
#include <interrupts/timer.h>
#define IDT_ENTRIES 256

extern uint64_t irq_stub_table[];

// ---------------------------
// NTux-OS: Global IDT and IDTR
// ---------------------------
static struct idt_entry idt[IDT_ENTRIES];
static struct idt_ptr   idt_descriptor;

// ---------------------------
// NTux-OS: Set one IDT entry (64-bit)
// ---------------------------
static void idt_set_entry(uint8_t index, uint64_t handler) {
    idt[index].offset_low  = (uint16_t)(handler & 0xFFFF);
    idt[index].selector    = 0x08;                   
    idt[index].ist         = 0;
    idt[index].type_attr   = IDT_PRESENT | IDT_INT_GATE;
    idt[index].offset_mid  = (uint16_t)((handler >> 16) & 0xFFFF);
    idt[index].offset_high = (uint32_t)(handler >> 32);
    idt[index].zero        = 0;
}

// ---------------------------
// NTux-OS: Initialize IDT
// ---------------------------
void idt_init(void) {
    // Clear all entries first
    for (int i = 0; i < IDT_ENTRIES; i++) {
        idt[i].offset_low   = 0;
        idt[i].selector     = 0;
        idt[i].ist          = 0;
        idt[i].type_attr    = 0;
        idt[i].offset_mid   = 0;
        idt[i].offset_high  = 0;
        idt[i].zero         = 0;
    }
    // Register IRQ handlers (from IRQ 32 to IRQ 47)
    for (int i = 0; i < 16; i++) {
        idt_set_entry(32 + i, irq_stub_table[i]);
    }
    
    kprint_ok("Registered IRQ handlers in IDT");

    // Set up IDT descriptor
    idt_descriptor.limit = sizeof(struct idt_entry) * IDT_ENTRIES - 1;
    idt_descriptor.base  = (uint64_t)&idt;

    // Load the IDT
    idt_load(&idt_descriptor);
}