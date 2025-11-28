#ifndef PCI_H
#define PCI_H

#include <stdint.h>

// PCI Config Space Register Offsets
#define PCI_VENDOR_ID            0x00
#define PCI_DEVICE_ID            0x02
#define PCI_COMMAND              0x04
#define PCI_STATUS               0x06
#define PCI_CLASS_CODE           0x0B
#define PCI_SUBCLASS             0x0A
#define PCI_PROG_IF              0x09
#define PCI_HEADER_TYPE          0x0E
#define PCI_BAR0                 0x10
#define PCI_BAR1                 0x14
#define PCI_BAR2                 0x18
#define PCI_BAR3                 0x1C
#define PCI_BAR4                 0x20
#define PCI_BAR5                 0x24
#define PCI_CAPABILITIES         0x34
#define PCI_INTERRUPT_LINE       0x3C

// Wichtige Class Codes für USB
#define PCI_CLASS_SERIAL         0x0C
#define PCI_SUBCLASS_USB         0x03
#define PCI_PROGIF_UHCI          0x00
#define PCI_PROGIF_OHCI          0x10
#define PCI_PROGIF_EHCI          0x20
#define PCI_PROGIF_XHCI          0x30

void pci_init(void);
uint32_t pci_read(uint32_t bus, uint32_t device, uint32_t function, uint32_t offset);
void pci_write(uint32_t bus, uint32_t device, uint32_t function, uint32_t offset, uint32_t value);
uint32_t pci_find_device(uint8_t class, uint8_t subclass, uint8_t prog_if);
uint32_t pci_get_bar(uint32_t pci_addr, int bar_index);
void pci_scan_all(void);

#endif