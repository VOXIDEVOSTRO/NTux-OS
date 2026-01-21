#ifndef ATA_H
#define ATA_H

#include <stdint.h>
#include <stdbool.h>
#include <drivers/fs/FAT/fat32.h>

#define ATA_SR_BSY              0x80
#define ATA_SR_DRDY             0x40
#define ATA_SR_DF               0x20
#define ATA_SR_DSC              0x10
#define ATA_SR_DRQ              0x08
#define ATA_SR_CORR             0x04
#define ATA_SR_IDX              0x02
#define ATA_SR_ERR              0x01

#define ATA_CMD_READ_PIO        0x20
#define ATA_CMD_READ_PIO_EXT    0x24
#define ATA_CMD_WRITE_PIO       0x30
#define ATA_CMD_WRITE_PIO_EXT   0x34
#define ATA_CMD_CACHE_FLUSH     0xE7
#define ATA_CMD_CACHE_FLUSH_EXT 0xEA
#define ATA_CMD_IDENTIFY        0xEC

#define ATA_REG_DATA            0x00
#define ATA_REG_ERROR           0x01
#define ATA_REG_FEATURES        0x01
#define ATA_REG_SECCOUNT0       0x02
#define ATA_REG_LBA0            0x03
#define ATA_REG_LBA1            0x04
#define ATA_REG_LBA2            0x05
#define ATA_REG_HDDEVSEL        0x06
#define ATA_REG_COMMAND         0x07
#define ATA_REG_STATUS          0x07
#define ATA_REG_SECCOUNT1       0x08
#define ATA_REG_LBA3            0x09
#define ATA_REG_LBA4            0x0A
#define ATA_REG_LBA5            0x0B
#define ATA_REG_CONTROL         0x206
#define ATA_REG_ALTSTATUS       0x206

#define ATA_SECTOR_SIZE         512

typedef struct {
    uint16_t flags;
    uint16_t unused1[9];
    char     serial[20];
    uint16_t unused2[3];
    char     firmware[8];
    char     model[40];
    uint16_t sectors_per_int;
    uint16_t unused3;
    uint16_t capabilities[2];
    uint16_t unused4[2];
    uint16_t valid_ext_data;
    uint16_t unused5[5];
    uint16_t size_of_rw_mult;
    uint16_t lba28_low;
    uint16_t lba28_high;
    uint16_t unused6[38];
    uint16_t lba48_0;
    uint16_t lba48_1;
    uint16_t lba48_2;
    uint16_t lba48_3;
    uint16_t unused7[152];
} __attribute__((packed)) ata_identify_t;

typedef struct {
    uint16_t io_base;
    uint16_t control_base;
    uint8_t  slave;
    bool     present;
    bool     lba48_supported;
    ata_identify_t identify;
} ata_device_t;

extern ata_device_t g_ata0;

bool ata_init(void);
bool ata_get_disk_ops(DiskOps* ops);
uint64_t ata_get_total_sectors(void);
uint64_t ata_get_total_space_bytes(void);

#endif
