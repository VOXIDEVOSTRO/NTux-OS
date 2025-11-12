#include "fb.h"
#include "kernel_res/fonts/font8x8/font8x8_basic.h"
#include <stdint.h>


void put_pixel_lim(volatile struct limine_framebuffer* fb, int x, int y, uint32_t color) {
    if (x < 0 || y < 0) return;
    if ((uint64_t)x >= fb->width || (uint64_t)y >= fb->height) return;

    uint8_t* pixel = (uint8_t*)fb->address + y * fb->pitch + x * (fb->bpp / 8);
    *(uint32_t*)pixel = color;
}

void clear_screen_lim(volatile struct limine_framebuffer* fb, uint32_t color) {
    for (uint64_t y = 0; y < fb->height; y++) {
        for (uint64_t x = 0; x < fb->width; x++) {
            put_pixel_lim(fb, x, y, color);
        }
    }
}
void draw_char_lim(volatile struct limine_framebuffer* fb, int x, int y, char c, uint32_t color) {
    for (int row = 0; row < 8; row++) {
        uint8_t bits = font8x8_basic[(int)c][row];
        for (int col = 0; col < 8; col++) {
            if (bits & (1 << col)) {
                put_pixel_lim(fb, x + col, y + row, color);
            }
        }
    }
}

void draw_text_lim(volatile struct limine_framebuffer* fb, int x, int y, const char* str, uint32_t color) {
    int cursor_x = x;
    while (*str) {
        if (*str == '\n') {
            cursor_x = x;
            y += 8;
        } else {
            draw_char_lim(fb, cursor_x, y, *str, color);
            cursor_x += 8;
        }
        str++;
    }
}




void draw_scaled_char_lim(volatile struct limine_framebuffer* fb, int x, int y, const char* char_lines[5], uint32_t color, int scale) {
    for (int row = 0; row < 5; row++) {
        const char* line = char_lines[row];
        for (int col = 0; line[col]; col++) {
            if (line[col] != ' ')
                for (int dy = 0; dy < scale; dy++)
                    for (int dx = 0; dx < scale; dx++)
                        put_pixel_lim(fb, x + col * scale + dx, y + row * scale + dy, color);
        }
    }
}

void draw_square_lim(volatile struct limine_framebuffer* fb, int x, int y, int size, uint32_t color) {
    for (int yy = 0; yy < size; yy++)
        for (int xx = 0; xx < size; xx++)
            put_pixel_lim(fb, x + xx, y + yy, color);
}


