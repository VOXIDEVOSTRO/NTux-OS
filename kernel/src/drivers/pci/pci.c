// drivers/pci/pci.c
#include <drivers/pci/pci.h>
#include <kernel_lib/io.h>
#include <drivers/framebuffer/kprint.h>

// PCI Configuration Space Ports
#define PCI_CONFIG_ADDRESS  0xCF8
#define PCI_CONFIG_DATA     0xCFC

// PCI Class Names (English)
static const char* pci_class_name(uint8_t class_code) {
    switch (class_code) {
        case 0x00: return "Unclassified Device";
        case 0x01: return "Mass Storage Controller";
        case 0x02: return "Network Controller";
        case 0x03: return "Display Controller";
        case 0x04: return "Multimedia Controller";
        case 0x05: return "Memory Controller";
        case 0x06: return "Bridge";
        case 0x07: return "Communication Controller";
        case 0x08: return "System Peripheral";
        case 0x09: return "Input Device Controller";
        case 0x0A: return "Docking Station";
        case 0x0B: return "Processor";
        case 0x0C: return "Serial Bus Controller";     // USB!
        case 0x0D: return "Wireless Controller";
        case 0x0E: return "Intelligent Controller";
        case 0x0F: return "Satellite Controller";
        case 0x10: return "Encryption Controller";
        case 0x11: return "Signal Processing Controller";
        default:   return "Unknown";
    }
}

static const char* usb_controller_type(uint8_t prog_if) {
    switch (prog_if) {
        case 0x00: return "UHCI";
        case 0x10: return "OHCI";
        case 0x20: return "EHCI";
        case 0x30: return "XHCI";
        case 0x80: return "USB (generic)";
        case 0xFE: return "USB Device";
        default:   return "Unknown USB";
    }
}

// Safe PCI scan – only Bus 0 (prevents faults on most systems)
void pci_scan_all(void) {
    kprint("╔═══════════════════════════════════════════════════════════╗\n");
    kprint("║                   PCI DEVICE SCAN                         ║\n");
    kprint("╚═══════════════════════════════════════════════════════════╝\n");

    int found = 0;

    // Only scan Bus 0 → this is safe and finds all common devices (including USB)
    for (uint32_t bus = 0; bus < 1; ++bus) {
        kprintf("Scanning Bus %u...\n", bus);

        for (uint32_t dev = 0; dev < 32; ++dev) {
            uint32_t data = pci_read(bus, dev, 0, 0x00);
            uint16_t vendor = data & 0xFFFF;
            uint16_t device_id = data >> 16;

            if (vendor == 0xFFFF || vendor == 0x0000) continue;

            found++;

            uint32_t class_data = pci_read(bus, dev, 0, 0x08);
            uint8_t class_code = class_data >> 24;
            uint8_t subclass   = (class_data >> 16) & 0xFF;
            uint8_t prog_if    = (class_data >> 8)  & 0xFF;
            uint8_t revision   = class_data & 0xFF;

            uint8_t header_type = (pci_read(bus, dev, 0, 0x0C) >> 16) & 0xFF;

            kprintf("%02x:%02x.%d  Vendor:%04x Device:%04x  Rev:%02x  %02x:%02x:%02x  %s",
                    bus, dev, 0, vendor, device_id, revision, class_code, subclass, prog_if,
                    pci_class_name(class_code));

            if (class_code == 0x0C && subclass == 0x03) {
                kprintf("  ← %s", usb_controller_type(prog_if));
            }

            kprint("\n");

            // Multi-function device?
            if (header_type & 0x80) {
                for (uint32_t func = 1; func < 8; ++func) {
                    uint32_t fdata = pci_read(bus, dev, func, 0x00);
                    if ((fdata & 0xFFFF) == 0xFFFF) continue;

                    uint16_t fvendor = fdata & 0xFFFF;
                    uint16_t fdevice = fdata >> 16;
                    uint32_t fclass = pci_read(bus, dev, func, 0x08);
                    uint8_t fclass_code = fclass >> 24;

                    kprintf("   └─ %02x:%02x.%d  Vendor:%04x Device:%04x  %s\n",
                            bus, dev, func, fvendor, fdevice, pci_class_name(fclass_code));
                }
            }
        }
    }

    kprintf("\nPCI scan complete: %d device(s) found.\n\n", found);
}

// Build PCI config address
static inline uint32_t pci_make_address(uint32_t bus, uint32_t dev, uint32_t func, uint32_t offset) {
    return 0x80000000u |
           (bus << 16) |
           (dev << 11) |
           (func << 8) |
           (offset & 0xFC);
}

// Read/Write functions
uint32_t pci_read(uint32_t bus, uint32_t dev, uint32_t func, uint32_t offset) {
    outl(PCI_CONFIG_ADDRESS, pci_make_address(bus, dev, func, offset));
    return inl(PCI_CONFIG_DATA);
}

void pci_write(uint32_t bus, uint32_t dev, uint32_t func, uint32_t offset, uint32_t value) {
    outl(PCI_CONFIG_ADDRESS, pci_make_address(bus, dev, func, offset));
    outl(PCI_CONFIG_DATA, value);
}

// PCI init test
void pci_init(void) {
    kprint("[PCI] Testing config space access... ");
    outl(PCI_CONFIG_ADDRESS, 0x80000000);
    if (inl(PCI_CONFIG_ADDRESS) == 0x80000000) {
        kprint_ok("");
    } else {
        kprint_error("PCI not available!");
    }
}

// Find device by class/subclass (also limited to Bus 0 for safety)
uint32_t pci_find_device(uint8_t class_code, uint8_t subclass, uint8_t prog_if) {
    for (uint32_t bus = 0; bus < 1; ++bus) {
        for (uint32_t dev = 0; dev < 32; ++dev) {
            uint32_t data = pci_read(bus, dev, 0, 0x00);
            if ((data & 0xFFFF) == 0xFFFF) continue;

            uint32_t class_data = pci_read(bus, dev, 0, 0x08);
            uint8_t cc = class_data >> 24;
            uint8_t sc = (class_data >> 16) & 0xFF;
            uint8_t pi = (class_data >> 8) & 0xFF;

            if (cc == class_code && sc == subclass && (prog_if == 0xFF || pi == prog_if)) {
                kprintf("[PCI] Found: %02x:%02x.0 Vendor:%04x Device:%04x → %s\n",
                        bus, dev, data & 0xFFFF, data >> 16,
                        (class_code == 0x0C && subclass == 0x03 && pi == 0x00) ? "UHCI" : "Device");
                return (bus << 16) | (dev << 8);
            }
        }
    }
    return 0;
}

// Get I/O BAR (UHCI uses BAR4)
uint32_t pci_get_bar(uint32_t pci_addr, int bar_index) {
    uint32_t bus = pci_addr >> 16;
    uint32_t dev = (pci_addr >> 8) & 0xFF;
    uint32_t bar = pci_read(bus, dev, 0, 0x10 + bar_index * 4);
    if (bar & 1) {
        return bar & 0xFFFFFFFC;  // I/O space
    }
    return 0;
}