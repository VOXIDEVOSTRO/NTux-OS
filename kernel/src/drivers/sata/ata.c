#include "ata.h"
#include <kernel_lib/io.h>
#include <libc/string.h>

#define ATA_TIMEOUT 2000000

ata_device_t g_ata0 = {
    .io_base         = 0x1F0,
    .control_base    = 0x3F6,
    .slave           = 0,
    .present         = false,
    .lba48_supported = false,
};

static void insw(uint16_t port, uint16_t* buffer, uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
        buffer[i] = inw(port);
    }
}

static void outsw(uint16_t port, const uint16_t* buffer, uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
        outw(port, buffer[i]);
    }
}

static void ata_io_wait(ata_device_t* dev) {
    inb(dev->control_base);
    inb(dev->control_base);
    inb(dev->control_base);
    inb(dev->control_base);
}

static bool ata_poll(ata_device_t* dev, bool check_error) {
    ata_io_wait(dev);
    uint8_t status;
    for (int i = 0; i < ATA_TIMEOUT; i++) {
        status = inb(dev->io_base + ATA_REG_STATUS);
        if ((status & ATA_SR_BSY) == 0) {
            break;
        }
    }
    status = inb(dev->io_base + ATA_REG_STATUS);
    if (status & ATA_SR_ERR) {
        return false;
    }
    if (status & ATA_SR_DF) {
        return false;
    }
    if (check_error && (status & ATA_SR_DRQ) == 0) {
        return false;
    }
    return true;
}

static void ata_soft_reset(ata_device_t* dev) {
    outb(dev->control_base, 0x04);
    ata_io_wait(dev);
    outb(dev->control_base, 0x00);
    ata_io_wait(dev);
    ata_poll(dev, false);
}

static bool ata_identify(ata_device_t* dev) {
    uint16_t bus = dev->io_base;
    uint8_t slavebit = dev->slave ? 0x10 : 0x00;
    ata_soft_reset(dev);
    outb(bus + ATA_REG_HDDEVSEL, 0xA0 | slavebit);
    ata_io_wait(dev);
    outb(bus + ATA_REG_SECCOUNT0, 0);
    outb(bus + ATA_REG_LBA0, 0);
    outb(bus + ATA_REG_LBA1, 0);
    outb(bus + ATA_REG_LBA2, 0);
    outb(bus + ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
    ata_io_wait(dev);
    uint8_t status = inb(bus + ATA_REG_STATUS);
    if (status == 0) {
        return false;
    }
    if (!ata_poll(dev, false)) {
        return false;
    }
    uint8_t cl = inb(bus + ATA_REG_LBA1);
    uint8_t ch = inb(bus + ATA_REG_LBA2);
    if (cl != 0 || ch != 0) {
        return false;
    }
    if (!ata_poll(dev, true)) {
        return false;
    }
    insw(bus + ATA_REG_DATA, (uint16_t*)&dev->identify, 256);
    uint8_t* ptr = (uint8_t*)dev->identify.model;
    for (int i = 0; i < 40; i += 2) {
        uint8_t tmp = ptr[i];
        ptr[i] = ptr[i + 1];
        ptr[i + 1] = tmp;
    }
    ptr = (uint8_t*)dev->identify.serial;
    for (int i = 0; i < 20; i += 2) {
        uint8_t tmp = ptr[i];
        ptr[i] = ptr[i + 1];
        ptr[i + 1] = tmp;
    }
    dev->lba48_supported = false;
    return true;
}

static bool ata_pio_read(ata_device_t* dev, uint64_t lba, uint8_t sector_count, uint8_t* target) {
    uint16_t bus = dev->io_base;
    uint8_t slavebit = dev->slave ? 0x10 : 0x00;
    bool use_lba48 = (lba > 0x0FFFFFFF) || dev->lba48_supported;
    outb(bus + ATA_REG_HDDEVSEL, 0xE0 | slavebit | (use_lba48 ? 0 : ((lba >> 24) & 0x0F)));
    if (use_lba48) {
        outb(bus + ATA_REG_SECCOUNT1, 0);
        outb(bus + ATA_REG_LBA3, (lba >> 24) & 0xFF);
        outb(bus + ATA_REG_LBA4, (lba >> 32) & 0xFF);
        outb(bus + ATA_REG_LBA5, (lba >> 40) & 0xFF);
    }
    outb(bus + ATA_REG_SECCOUNT0, sector_count);
    outb(bus + ATA_REG_LBA0, lba & 0xFF);
    outb(bus + ATA_REG_LBA1, (lba >> 8) & 0xFF);
    outb(bus + ATA_REG_LBA2, (lba >> 16) & 0xFF);
    if (use_lba48) {
        outb(bus + ATA_REG_SECCOUNT1, 0);
        outb(bus + ATA_REG_LBA3, (lba >> 24) & 0xFF);
        outb(bus + ATA_REG_LBA4, (lba >> 32) & 0xFF);
        outb(bus + ATA_REG_LBA5, (lba >> 40) & 0xFF);
    }
    outb(bus + ATA_REG_COMMAND, use_lba48 ? ATA_CMD_READ_PIO_EXT : ATA_CMD_READ_PIO);
    for (uint8_t i = 0; i < sector_count; i++) {
        if (!ata_poll(dev, true)) {
            return false;
        }
        insw(bus + ATA_REG_DATA, (uint16_t*)target, ATA_SECTOR_SIZE / 2);
        target += ATA_SECTOR_SIZE;
    }
    return true;
}

static bool ata_pio_write(ata_device_t* dev, uint64_t lba, uint8_t sector_count, const uint8_t* source) {
    uint16_t bus = dev->io_base;
    uint8_t slavebit = dev->slave ? 0x10 : 0x00;
    bool use_lba48 = (lba > 0x0FFFFFFF) || dev->lba48_supported;
    outb(bus + ATA_REG_HDDEVSEL, 0xE0 | slavebit | (use_lba48 ? 0 : ((lba >> 24) & 0x0F)));
    if (use_lba48) {
        outb(bus + ATA_REG_SECCOUNT1, 0);
        outb(bus + ATA_REG_LBA3, (lba >> 24) & 0xFF);
        outb(bus + ATA_REG_LBA4, (lba >> 32) & 0xFF);
        outb(bus + ATA_REG_LBA5, (lba >> 40) & 0xFF);
    }
    outb(bus + ATA_REG_SECCOUNT0, sector_count);
    outb(bus + ATA_REG_LBA0, lba & 0xFF);
    outb(bus + ATA_REG_LBA1, (lba >> 8) & 0xFF);
    outb(bus + ATA_REG_LBA2, (lba >> 16) & 0xFF);
    if (use_lba48) {
        outb(bus + ATA_REG_SECCOUNT1, 0);
        outb(bus + ATA_REG_LBA3, (lba >> 24) & 0xFF);
        outb(bus + ATA_REG_LBA4, (lba >> 32) & 0xFF);
        outb(bus + ATA_REG_LBA5, (lba >> 40) & 0xFF);
    }
    outb(bus + ATA_REG_COMMAND, use_lba48 ? ATA_CMD_WRITE_PIO_EXT : ATA_CMD_WRITE_PIO);
    for (uint8_t i = 0; i < sector_count; i++) {
        if (!ata_poll(dev, true)) {
            return false;
        }
        outsw(bus + ATA_REG_DATA, (const uint16_t*)source, ATA_SECTOR_SIZE / 2);
        source += ATA_SECTOR_SIZE;
    }
    outb(bus + ATA_REG_COMMAND, use_lba48 ? ATA_CMD_CACHE_FLUSH_EXT : ATA_CMD_CACHE_FLUSH);
    ata_poll(dev, false);
    return true;
}

static bool ata_disk_read(uint8_t* buf, uint32_t sect) {
    return ata_pio_read(&g_ata0, sect, 1, buf);
}

static bool ata_disk_write(const uint8_t* buf, uint32_t sect) {
    return ata_pio_write(&g_ata0, sect, 1, buf);
}

bool ata_init(void) {
    memset(&g_ata0.identify, 0, sizeof(g_ata0.identify));
    g_ata0.present = ata_identify(&g_ata0);
    return g_ata0.present;
}

bool ata_get_disk_ops(DiskOps* ops) {
    if (!g_ata0.present) {
        return false;
    }
    if (!ops) {
        return false;
    }
    ops->read = ata_disk_read;
    ops->write = ata_disk_write;
    return true;
}

uint64_t ata_get_total_sectors(void) {
    if (!g_ata0.present) {
        return 0;
    }
    ata_identify_t* id = &g_ata0.identify;
    if (g_ata0.lba48_supported) {
        uint64_t lba48 = ((uint64_t)id->lba48_3 << 48) |
                         ((uint64_t)id->lba48_2 << 32) |
                         ((uint64_t)id->lba48_1 << 16) |
                         (uint64_t)id->lba48_0;
        if (lba48 > 0) {
            return lba48;
        }
    }
    uint32_t lba28 = ((uint32_t)id->lba28_high << 16) | id->lba28_low;
    return lba28;
}

uint64_t ata_get_total_space_bytes(void) {
    return ata_get_total_sectors() * ATA_SECTOR_SIZE;
}
