#ifndef FB_H
#define FB_H

#include <stdint.h>
#include <limine.h>

extern volatile struct limine_framebuffer_request framebuffer_request;

void put_pixel_lim(volatile struct limine_framebuffer* fb, int x, int y, uint32_t color);
void draw_char_lim(volatile struct limine_framebuffer* fb, int x, int y, char c, uint32_t color);
void draw_text_lim(volatile struct limine_framebuffer* fb, int x, int y, const char* str, uint32_t color);
 void clear_screen_lim(volatile struct limine_framebuffer* fb, uint32_t color);

int letter_index(char c);
void draw_scaled_char_lim(volatile struct limine_framebuffer* fb, int x, int y, const char* char_lines[5], uint32_t color, int scale);
void draw_square_lim(volatile struct limine_framebuffer* fb, int x, int y, int size, uint32_t color);
void draw_scaled_text_lim(volatile struct limine_framebuffer* fb, int x, int y, const char* str, uint32_t color, int scale);

#endif
