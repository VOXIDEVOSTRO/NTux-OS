#include <operators/power.h>
#include <drivers/audio/audio.h>
#include <stdint.h>

static inline void outl(uint16_t port, uint32_t val) { asm volatile ("outl %0, %1" : : "a"(val),  "Nd"(port)); }
static uint16_t acpi_pm1a_cnt = 0;
static uint16_t acpi_pm1b_cnt = 0;
static uint16_t acpi_slp_typa = 0;
static uint16_t acpi_slp_typb = 0;

void power_reboot(void)
{
    play_shutdown_sound();
    uint8_t status;
    do {
        status = inb(0x64);
    } while (status & 0x02);

    outb(0x64, 0xFE);
    for (;;) {
        asm volatile("hlt");
    }
}

extern void (*limine_shutdown_callback)(void);

void power_shutdown(void)
{
    kprint("powering off\n");
    play_shutdown_sound();
    if (limine_shutdown_callback)
    {
        limine_shutdown_callback();
        __builtin_unreachable();
    }

    uint16_t val = acpi_slp_typa | (1 << 13); 
        outw(acpi_pm1a_cnt, val);
        if (acpi_pm1b_cnt) {
            val = acpi_slp_typb | (1 << 13);
            outw(acpi_pm1b_cnt, val);
        }
    

    const uint16_t acpi_ports[] = {
        0xB004,  
        0x604,   
        0x4004,  
        0x1004,  
        0x2004,  
        0x5004,  
        0xB000,  
        0x2F04   
    };

    for (int i = 0; i < sizeof(acpi_ports)/sizeof(acpi_ports[0]); i++) {
        outw(acpi_ports[i], 0x2000);
        outl(acpi_ports[i], 0x2000); 
        outw(acpi_ports[i] + 4, 0x2000);
    }
    outb(0xB2, 0x01);
    outb(0xB2, 0x00);
    outw(0xB004, 0x2000);
    outw(0x604, 0x2000);
    outw(0xB2, 0x2000);
    outb(0xB3, 0x01);
    /*
    __asm__ volatile (
        "mov $0x1000, %ax\n"
        "mov %ax, %dx\n"
        "mov $0x5307, %ax\n"
        "mov $0x0001, %bx\n"
        "mov $0x0003, %cx\n"
        "int $0x15\n"
        ::: "eax","ebx","ecx","edx"
    );*/

    outb(0x64, 0xFE);   
}
