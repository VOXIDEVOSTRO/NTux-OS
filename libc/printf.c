#include <stdbool.h>
#include <stdint.h>

#include <libc/printf.h>
#include <drivers/framebuffer/kprint.h>

void printf(const char* format, ...){
    kprintf(format);
}