#ifndef POWER_H
#define POWER_H

#include <stdint.h>

typedef struct {
    char signature[8];  
    uint8_t checksum;
    char oem_id[6];
    uint8_t revision;
    uint32_t rsdt_address;
    uint32_t length;
    uint64_t xsdt_address;
    uint8_t extended_checksum;
    uint8_t reserved[3];
} __attribute__((packed)) acpi_rsdp_t;

typedef struct {
    uint32_t signature;
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    uint8_t oem_id[6];
    uint8_t oem_table_id[8];
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
    uint32_t pm1a_control_block;
    uint32_t pm1b_control_block;
    uint32_t pm_timer_block;
    // Weitere Felder hier...
} __attribute__((packed)) acpi_fadt_t;

void power_shutdown(void);
void power_reboot(void);
acpi_rsdp_t* find_rsdp();
int init_acpi();

#endif