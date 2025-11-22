#ifndef PIC_H
#define PIC_H

#include <stdint.h>

// ---------------------------
// NTux-OS: PIC I/O port addresses and command words
// ---------------------------
#define MASTER_PIC_CMD    0x20
#define MASTER_PIC_DATA   0x21
#define SLAVE_PIC_CMD     0xA0
#define SLAVE_PIC_DATA    0xA1

#define PIC_ICW1_ICW4 0x01 
#define PIC_ICW1_SINGLE 0x02
#define PIC_ICW1_INTERVAL4 0x04
#define PIC_ICW1_LEVEL 0x08
#define PIC_ICW4_8086 0x01
#define PIC_ICW4_MASTER 0x02 
#define PIC_ICW4_SLAVE 0x01

void pic_init(void);
void pic_send_eoi(uint8_t irq);

#endif