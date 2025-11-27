#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

void keyboard_init();
int keyboard_getchar(char* c);
void keyboard_poll();
int keyboard_is_key_pressed(uint8_t key) ;
void keyboard_irq_handler(void);

extern uint8_t shift_pressed;
extern uint8_t ctrl_pressed;
extern uint8_t alt_pressed;

#endif