#ifndef KPRINT_H
#define KPRINT_H

#include <stdint.h>
#include "cursor.h"
#include "fb.h"

typedef struct {
    volatile struct limine_framebuffer* fb;
    cursor_t* cursor;
    uint32_t color;
} kprint_t;

extern kprint_t g_printer;

void itoa(int num, char* str, int base);

void init_kprint_global(volatile struct limine_framebuffer* fb, cursor_t* cursor, uint32_t color);
void kprint(const char* text);
void kprintcolor(const char* text, uint32_t color);
void kprint_int(int num);
void kprint_uint(uint32_t num);
void kprinthex(uint32_t num);
void kprintcolor_int32(int num, uint32_t color);
void kprintcolor_uint32(uint32_t num, uint32_t color);
void kprintcolorhex(uint32_t num, uint32_t color);
void kprint_ok(const char* text);
void kprint_error(const char* text);

#endif
