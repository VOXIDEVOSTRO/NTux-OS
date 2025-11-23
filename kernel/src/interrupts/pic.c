#include <interrupts/pic.h>
#include <kernel_lib/io.h>


void pic_send_command(uint8_t cmd,uint16_t port){
    outb(port,cmd);
}

void pic_send_data(uint8_t data,uint16_t port){
    outb(port,data);
}

void pic_init(void) {
    pic_send_command(0x11, MASTER_PIC_CMD);  
    pic_send_command(0x11, SLAVE_PIC_CMD);   
    pic_send_data(0x20, MASTER_PIC_DATA);  
    pic_send_data(0x28, SLAVE_PIC_DATA);   
    pic_send_data(0x04, MASTER_PIC_DATA);  
    pic_send_data(0x02, SLAVE_PIC_DATA);   
    pic_send_data(0x01, MASTER_PIC_DATA);  
    pic_send_data(0x01, SLAVE_PIC_DATA);    
    pic_send_data(0xFE, MASTER_PIC_DATA);
    pic_send_data(0xFF, SLAVE_PIC_DATA); 
}

void pic_send_eoi(uint8_t irq) {
    if (irq >= 8) {
        outb(0x20, SLAVE_PIC_CMD);
    }
    outb(0x20, MASTER_PIC_CMD);
}