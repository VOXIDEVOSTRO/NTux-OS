#include <interrupts/isr.h>
#include <interrupts/idt.h>
#include <drivers/framebuffer/kprint.h>

void isr_handle_error(void) {
    trigger_blue_screen();
}

