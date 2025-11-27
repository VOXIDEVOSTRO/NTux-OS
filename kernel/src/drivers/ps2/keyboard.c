#include "keyboard.h"
#include <kernel_lib/io.h>
#include "ringbuffer.h"
#include "scancode_table_de.h"


ringbuffer_t kb_buffer;
uint8_t shift_pressed = 0;
uint8_t ctrl_pressed  = 0;
uint8_t alt_pressed   = 0;

extern char scancode_ascii[128];
extern char scancode_ascii_shift[128];

void keyboard_init() {
    rb_init(&kb_buffer);
}

void keyboard_poll() {
    uint8_t status = inb(0x64);  
    if(!(status & 1)) return;    

    uint8_t scancode = inb(0x60);

    uint8_t released = scancode & 0x80;
    uint8_t key = scancode & 0x7F;

    if(key == 0x2A || key == 0x36) shift_pressed = !released; 
    if(key == 0x1D) ctrl_pressed  = !released; 
    if(key == 0x38) alt_pressed   = !released; 

    if(!released) {
        char c = shift_pressed ? scancode_ascii_shift[key] : scancode_ascii[key];
        if(c) rb_put(&kb_buffer, c);
    }
}

int keyboard_getchar(char* c) {
    return rb_get(&kb_buffer, c);
}

int keyboard_is_key_pressed(uint8_t key) {
    if (key == SCANCODE_LEFT_SHIFT || key == SCANCODE_RIGHT_SHIFT) {
        return shift_pressed;
    }
    if (key == SCANCODE_CTRL) {
        return ctrl_pressed;
    }
    if (key == SCANCODE_ALT) {
        return alt_pressed;
    }
    

    char c = shift_pressed ? scancode_ascii_shift[key] : scancode_ascii[key];
    return (c != 0);  
}

void keyboard_irq_handler(void) {
    keyboard_poll();       
}