#include <operators/power.h>
#include <drivers/audio/audio.h>
#include <stdint.h>
#include <stdbool.h>
#include <interrupts/interrupts.h>
#include <stddef.h>


#define RSDP_SIGNATURE "RSD PTR "
#define FADT_SIGNATURE "FACP"

static acpi_rsdp_t* rsdp = {0};
static acpi_fadt_t* fadt = {0};

static uint16_t acpi_pm1a_cnt = 0;
static uint16_t acpi_pm1b_cnt = 0;
static uint16_t acpi_slp_typa = 0;
static uint16_t acpi_slp_typb = 0;


static inline uint16_t inw(uint16_t port) {
    uint16_t ret;
    __asm__ volatile ("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outw(uint16_t port, uint16_t val) {
    __asm__ volatile ("outw %0, %1" : : "a"(val), "Nd"(port));
}


extern void (*limine_shutdown_callback)(void);

acpi_rsdp_t* find_rsdp() {
    uint8_t *memory = (uint8_t*) 0x000E0000;
    uint8_t *end    = (uint8_t*) 0x00100000;

    const char sig[8] = {'R','S','D',' ','P','T','R',' '};

    for (uint8_t *p = memory; p + 8 <= end; p += 16) {
        bool match =
            p[0]==sig[0] && p[1]==sig[1] && p[2]==sig[2] && p[3]==sig[3] &&
            p[4]==sig[4] && p[5]==sig[5] && p[6]==sig[6] && p[7]==sig[7];
        if (match) {
            return (acpi_rsdp_t*) p;
        }
    }
    return NULL;
}

int init_acpi() {
    rsdp = find_rsdp();
    if (!rsdp) {
        return -1;
    }

    uint64_t xsdt_address = rsdp->xsdt_address;
    if (xsdt_address != 0) {

        uint64_t* xsdt = (uint64_t*) xsdt_address;
        acpi_fadt_t* maybe_fadt = (acpi_fadt_t*) xsdt[0];
        if (maybe_fadt && maybe_fadt->signature == *(const uint32_t*)"FACP") {
            fadt = maybe_fadt;
        }
    }
    if (!fadt) {
        uint32_t* rsdt = (uint32_t*) rsdp->rsdt_address;
        acpi_fadt_t* maybe_fadt = (acpi_fadt_t*) rsdt[0];
        if (maybe_fadt && maybe_fadt->signature == *(const uint32_t*)"FACP") {
            fadt = maybe_fadt;
        }
    }

    if (!fadt || fadt->signature != *(const uint32_t*)"FACP") {
        return -2;
    }

    acpi_pm1a_cnt = (uint16_t)fadt->pm1a_control_block;
    acpi_pm1b_cnt = (uint16_t)fadt->pm1b_control_block;

    if (acpi_pm1a_cnt == 0) {
        return -3;
    }


    return 0;
}


void power_reboot(void) {


    play_shutdown_sound();
    uint8_t status;
    int retries = 10;
    do {
        status = inb(0x64);
        if (--retries == 0) {
            return;
        }
    } while (status & 0x02);

    outb(0x64, 0xFE);
    for (;;) {
        asm volatile("hlt");
    }
}

void power_shutdown(void) {

    uint16_t slp_typ_a = acpi_slp_typa ? acpi_slp_typa : 5;
    uint16_t slp_typ_b = acpi_slp_typb ? acpi_slp_typb : slp_typ_a;
    const uint16_t SLP_TYP_SHIFT = 10;
    const uint16_t SLP_EN_BIT    = (1u << 13);
    const uint16_t SLP_TYP_MASK  = (7u << SLP_TYP_SHIFT);

    if (acpi_pm1a_cnt) {
        uint16_t v = inw(acpi_pm1a_cnt);
        v = (v & ~SLP_TYP_MASK) | (slp_typ_a << SLP_TYP_SHIFT);
        outw(acpi_pm1a_cnt, v);
        outw(acpi_pm1a_cnt, v | SLP_EN_BIT);
    }
    if (acpi_pm1b_cnt) {
        uint16_t v = inw(acpi_pm1b_cnt);
        v = (v & ~SLP_TYP_MASK) | (slp_typ_b << SLP_TYP_SHIFT);
        outw(acpi_pm1b_cnt, v);
        outw(acpi_pm1b_cnt, v | SLP_EN_BIT);
    }

    __asm__ volatile ("cli; hlt");
    for (;;){
            __asm__ volatile ("hlt");
    }
}
