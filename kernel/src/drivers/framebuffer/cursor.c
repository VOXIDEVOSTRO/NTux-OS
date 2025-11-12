#include "cursor.h"
#include "fb.h"
#include <stdint.h>
#include <stddef.h>

void init_cursor(cursor_t* cursor, int screen_width, int screen_height) {
    cursor->x = 0;
    cursor->y = 0;
    cursor->char_width = 8;
    cursor->char_height = 8;
    cursor->screen_width = screen_width;
    cursor->screen_height = screen_height;
}

void put_char_with_cursor_lim(volatile struct limine_framebuffer* fb, cursor_t* cursor, char c, uint32_t color) {
    draw_char_lim(fb, cursor->x, cursor->y, c, color);

    cursor->x += cursor->char_width;

    if (cursor->x + cursor->char_width > cursor->screen_width) {
        cursor->x = 0;
        cursor->y += cursor->char_height;
    }

    if (cursor->y + cursor->char_height > cursor->screen_height) {
        int row_pixels = cursor->char_height;
        int col_pixels = cursor->screen_width;


        for (uint64_t y = 0; y < fb->height - row_pixels; y++) {
            for (uint64_t x = 0; x < fb->width; x++) {
                uint32_t* dest = (uint32_t*)((uintptr_t)fb->address + y * fb->pitch + x * (fb->bpp / 8));
                uint32_t* src  = (uint32_t*)((uintptr_t)fb->address + (y + row_pixels) * fb->pitch + x * (fb->bpp / 8));
                *dest = *src;
            }
        }

        for (uint64_t y = fb->height - row_pixels; y < fb->height; y++)
            for (uint64_t x = 0; x < fb->width; x++)
                put_pixel_lim(fb, x, y, COLOR_BLACK);

        cursor->y -= row_pixels;
    }
}


void render_cursor_lim(volatile struct limine_framebuffer* fb, cursor_t* cursor, uint32_t color) {
    for (int y = 0; y < cursor->char_height; y++) {
        for (int x = 0; x < cursor->char_width; x++) {
            uint32_t* pixel = (uint32_t*)((uintptr_t)fb->address + (cursor->y + y) * fb->pitch + (cursor->x + x) * (fb->bpp / 8));
            *pixel = color;
        }
    }
}

void clear_cursor_lim(volatile struct limine_framebuffer* fb, cursor_t* cursor) {
    for (int y = 0; y < cursor->char_height; y++) {
        for (int x = 0; x < cursor->char_width; x++) {
            uint32_t* pixel = (uint32_t*)((uintptr_t)fb->address + (cursor->y + y) * fb->pitch + (cursor->x + x) * (fb->bpp / 8));
            *pixel = COLOR_BLACK;
        }
    }
}
