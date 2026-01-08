#ifndef MOUSE_H
#define MOUSE_H

#include <stdbool.h>
#include <limine.h>
#include <stdint.h>

//monitor size
static int fb_width;
static int fb_height;

//mouse position and buttons 
extern int mouse_X;
extern int mouse_y;
extern int mouse_left;
extern int mouse_right;

//mouse initzialition
bool mouse_init(void);

//poll function will later be removed by using the irq but im to lazy
void mouse_poll(void);

//getter functions 
int mouse_get_x(void);
int mouse_get_y(void);
bool mouse_left_pressed(void);
bool mouse_right_pressed(void);
int mouse_get_scroll(void); 

//test function
void draw_mouse_cursor(volatile struct limine_framebuffer* fb);


bool mouse_data();

#endif