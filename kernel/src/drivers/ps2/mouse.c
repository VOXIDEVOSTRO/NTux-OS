#include <drivers/ps2/mose.h>
#include <kernel_lib/io.h>
#include <drivers/framebuffer/fb.h>
#include <drivers/ps2/ringbuffer.h>

#define PS2_STATUS 0x64
#define PS2_DATA 0x60

//intern values 
static uint8_t mouse_cycle = 0;
static uint8_t mouse_bytes[4];//4 bytes for last scroll

int mouse_X = 100;
int mouse_y = 100;
static int mouse_scroll = 0;
int mouse_left = 0;
int mouse_right = 0;


// --- Helping functions :D ---



static inline int mouse_has_data() {
    return inb(PS2_STATUS) & 1;
}

static inline uint8_t mouse_read() {
    return inb(PS2_DATA);
}

//initzialition
bool mouse_init(void){
    //enable mouse device
    outb(PS2_STATUS,0xA8);
    //enable streaming mode
    outb(PS2_STATUS,0xD4);
    outb(PS2_DATA,0xF4);

    uint8_t ack = 0;
    for(int i=0;i<1000;i++) {
        if(mouse_has_data()) {
            ack = mouse_read();
            break;
        }
    }

    if(ack != 0xFA) {
        kprint("No PS/2 mouse detected!\n");
        return false;
    }

    return true;
    //mouse init is now completet :D
}

//polling
void mouse_poll(){
    //if it does not have data it just return if it have data then it will get the data lol
    if (!mouse_has_data()) return;
    uint8_t data = mouse_read();

switch(mouse_cycle) {
        case 0:
            if (!(data & 0x08)) return; // Bit 3 must get setted
            mouse_bytes[0] = data;
            mouse_cycle = 1;
            break;
        case 1:
            mouse_bytes[1] = data;
            mouse_cycle = 2;
            break;
        case 2:
            mouse_bytes[2] = data;
            mouse_cycle = 3;
            break;
        case 3:
            mouse_bytes[3] = data;
            mouse_cycle = 0;

            int dx = (int8_t)mouse_bytes[1];
            int dy = (int8_t)mouse_bytes[2];
            int dz = (int8_t)mouse_bytes[3]; // scrolling

            mouse_X += dx;
            mouse_y -= dy;

            if(mouse_X < 0) mouse_X = 0;
            if(mouse_y < 0) mouse_y = 0;
            if(mouse_X > fb_width-1) mouse_X = fb_width-1;
            if(mouse_y > fb_height-1) mouse_y = fb_height-1;

            mouse_left  = mouse_bytes[0] & 1;
            mouse_right = mouse_bytes[0] & 2;
            mouse_scroll = dz;

            break;
    }
}


// for now a test function 
void draw_mouse_cursor(volatile struct limine_framebuffer* fb) {
    fb_width =  fb->width;
    fb_height =  fb->height;
    put_pixel_lim(fb,mouse_X, mouse_y, 0xFFFFFF);
    put_pixel_lim(fb,mouse_X+1, mouse_y, 0xFFFFFF);
    put_pixel_lim(fb,mouse_X-1, mouse_y, 0xFFFFFF);
    put_pixel_lim(fb,mouse_X, mouse_y+1, 0xFFFFFF);
    put_pixel_lim(fb,mouse_X, mouse_y-1, 0xFFFFFF);
}

// some getters 
int mouse_get_x(void) { return mouse_X; }
int mouse_get_y(void) { return mouse_y; }
bool mouse_left_pressed(void) { return mouse_left != 0; }
bool mouse_right_pressed(void) { return mouse_right != 0; }
int mouse_get_scroll(void) { return mouse_scroll; }
bool mouse_data(){
    return mouse_has_data();
}