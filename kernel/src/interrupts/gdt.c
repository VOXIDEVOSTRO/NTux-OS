#include "gdt.h"
#include <stdint.h>

static struct gdt_entry gdt[GDT_ENTRIES];
static struct gdt_ptr   gdt_ptr;
static struct tss_entry tss;

static void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran)
{
    gdt[num].base_low     = (base & 0xFFFF);
    gdt[num].base_middle  = (base >> 16) & 0xFF;
    gdt[num].base_high    = (base >> 24) & 0xFF;
    gdt[num].limit_low    = (limit & 0xFFFF);
    gdt[num].granularity = (limit >> 16) & 0x0F;
    gdt[num].granularity |= gran & 0xF0;
    gdt[num].access       = access;
}

void gdt_init(void)
{
    gdt_ptr.limit = sizeof(gdt) - 1;
    gdt_ptr.base  = (uint64_t)&gdt;

    /* Null descriptor */
    gdt_set_gate(GDT_NULL,   0, 0, 0, 0);

    /* 64-bit code segment */
    gdt_set_gate(GDT_CODE64, 0, 0xFFFFF, 0x9A, 0xA0);

    /* 64-bit data segment */
    gdt_set_gate(GDT_DATA64, 0, 0xFFFFF, 0x92, 0xA0);

    /* Clear TSS */
    for (int i = 0; i < sizeof(tss); i++) {
        ((uint8_t*)&tss)[i] = 0;
    }

    /* Set kernel stack for ring 0 */
    __asm__ volatile ("mov %%rsp, %0" : "=r"(tss.rsp0));

    /* No I/O permission bitmap */
    tss.iomap_base = sizeof(tss);

    /* TSS descriptor (uses two entries: 3 and 4) */
    uint64_t tss_base  = (uint64_t)&tss;
    uint32_t tss_limit = sizeof(tss) - 1;

    gdt[GDT_TSS].limit_low    = tss_limit & 0xFFFF;
    gdt[GDT_TSS].base_low     = tss_base & 0xFFFF;
    gdt[GDT_TSS].base_middle  = (tss_base >> 16) & 0xFF;
    gdt[GDT_TSS].access       = 0xE9;  /* 64-bit TSS, present */
    gdt[GDT_TSS].granularity  = (tss_limit >> 16) & 0x0F;
    gdt[GDT_TSS].base_high    = (tss_base >> 24) & 0xFF;

    /* Upper 32 bits of TSS base go into next entry */
    gdt_set_gate(6, 0, 0xFFFFF, 0xF2, 0xA0);
    gdt_set_gate(5, 0, 0xFFFFF, 0xFA, 0xA0);
    *(uint64_t*)&gdt[GDT_TSS + 1] = tss_base >> 32;

    /* Load GDT */
    gdt_load(&gdt_ptr);

    /* Load task register */
    __asm__ volatile ("ltr %0" : : "r"((uint16_t)(GDT_TSS * 8)));
}