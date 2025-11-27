#include "kprint.h"

kprint_t g_printer;

void itoa(int num, char* str, int base) {
    int i = 0;
    int isNegative = 0;
    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return;
    }
    if (num < 0 && base == 10) {
        isNegative = 1;
        num = -num;
    }
    while (num != 0) {
        int rem = num % base;
        str[i++] = '0';
        str[i] = '\0';
        return;
    }
    if (num < 0 && base == 10) {
        isNegative = 1;
        num = -num;
    }
    while (num != 0) {
        int rem = num % base;
        str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
        num = num / base;
    }
    if (isNegative) {
        str[i++] = '-';
    }
    str[i] = '\0'; 
    int start = 0;
    int end = i - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}


void init_kprint_global(volatile struct limine_framebuffer* fb, cursor_t* cursor, uint32_t color) {
    g_printer.fb = fb;
    g_printer.cursor = cursor;
    g_printer.color = color;
}

void kprint(const char* text) {
    const char* ptr = text;
    while (*ptr) {
        if (*ptr == '\n') {
            g_printer.cursor->x = 0;
            g_printer.cursor->y += g_printer.cursor->char_height;
        } else {
            put_char_with_cursor_lim(g_printer.fb, g_printer.cursor, *ptr, g_printer.color);
        }
        ptr++;
    }
}

void kprintcolor(const char* text,uint32_t color){
        const char* ptr = text;
    while (*ptr) {
        if (*ptr == '\n') {
            g_printer.cursor->x = 0;
            g_printer.cursor->y += g_printer.cursor->char_height;
        } else {
            put_char_with_cursor_lim(g_printer.fb, g_printer.cursor, *ptr, color);
        }
        ptr++;
    }
}

void kprint_int(int num) {
    char buffer[20]; 
    itoa(num, buffer, 10); 
    kprint(buffer); 
}

void kprint_uint(uint32_t num) {
    char buffer[20]; 
    itoa(num, buffer, 10); 
    kprint(buffer); 
}

void kprinthex(uint32_t num) {
    char buffer[20];
    itoa(num, buffer, 16); 
    kprint("0x");
    kprint(buffer);
}


void kprintcolor_int32(int num, uint32_t color) {
    char buffer[20];
    itoa(num, buffer, 10); 
    kprintcolor(buffer, color); 
}

void kprintcolor_uint32(uint32_t num, uint32_t color) {
    char buffer[20];
    itoa(num, buffer, 10); 
    kprintcolor(buffer, color); 
}


void kprintcolorhex(uint32_t num, uint32_t color) {
    char buffer[20];
    itoa(num, buffer, 16); 
    kprintcolor("0x", color); 
    kprintcolor(buffer, color); 
}

void kprint_ok(const char* text){
    kprint(text);
    kprint(" ");
    kprint("[");
    kprintcolor("  OK  ", COLOR_GREEN);
    kprint("]\n");
}


void kprint_error(const char* text){
    kprint(text);
    kprint(" ");
    kprint("[");
    kprintcolor(" ERROR ", COLOR_RED);
    kprint("]\n");
}

void trigger_blue_screen(void){
    clear_screen_lim(g_printer.fb, COLOR_LIGHT_BLUE_SCREEN_BG);
    g_printer.cursor->x = 3;
    g_printer.cursor->y = 10;
    kprint(":(\n");
    kprint("\n\n");
    kprint("A critical error has occurred. System halted.\ns");
    kprint_error("System will not continue.");

    for(;;){
        __asm__ volatile("hlt");
    }
}
