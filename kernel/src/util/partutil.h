#ifndef PART_H
#define PART_H

#include <stdint.h>
#include <stdbool.h>
#include <drivers/sata/ata.h>

typedef struct {
    uint8_t boot;
    uint8_t chs_start[3];
    uint8_t type;
    uint8_t chs_end[3];
    uint32_t lba_start;
    uint32_t sectors;
} __attribute__((packed)) part_entry_t;

void dump_partitions(DiskOps* ops);

#endif // PART_H