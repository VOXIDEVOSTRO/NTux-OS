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




void draw_scaled_char_lim(volatile struct limine_framebuffer* fb, int x, int y, const char* char_lines[8], uint32_t color, int scale) {
    for (int row = 0; row < 8; row++) {   
        uint8_t bits = font8x8_basic[(int)(*char_lines)][row];
        for (int col = 0; col < 8; col++) {  
            if (bits & (1 << col)) {  
            for (int dy = 0; dy < scale; dy++) {
                    for (int dx = 0; dx < scale; dx++) {
                        put_pixel_lim(fb, x + col * scale + dx, y + row * scale + dy, color);
                    }
                }
            }
        }
    }
}
void draw_scaled_text_lim(volatile struct limine_framebuffer* fb, int x, int y, const char* str, uint32_t color, int scale) {
    int cursor_x = x;
    while (*str) {
        if (*str == '\n') {  
            cursor_x = x;
            y += 8 * scale;  
        } else {
            draw_scaled_char_lim(fb, cursor_x, y, font8x8_basic[(int)(*str)], color, scale);  
            cursor_x += 8 * scale;  
        }
        str++;
    }
}


void draw_square_lim(volatile struct limine_framebuffer* fb, int x, int y, int size, uint32_t color) {
    for (int yy = 0; yy < size; yy++)
        for (int xx = 0; xx < size; xx++)
            put_pixel_lim(fb, x + xx, y + yy, color);
}


void draw_image_from_uint64_t(volatile struct limine_framebuffer* fb, uint64_t* background, int img_width, int img_height) {
    // Size of framebuffer
    int fb_width = fb->width;
    int fb_height = fb->height;

    // Going through every pixel in the framebuffer
    for (int y = 0; y < fb_height; y++) {
        for (int x = 0; x < fb_width; x++) {
            // Make sure we don't go beyond the bounds of the original image
            if (x < img_width && y < img_height) {
                // Get the color from the background array
                uint64_t color = background[y * img_width + x];

                // Draw the pixel to the framebuffer
                put_pixel_lim(fb, x, y, color);
            }
        }
    }
}
