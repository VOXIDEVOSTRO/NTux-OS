#include <interrupts/irq.h>
#include <interrupts/pic.h>
#include <drivers/framebuffer/kprint.h>

extern uint64_t irq_stub_table[];

void (*irq_handlers[16])(void) = {0};  

void irq_register_handler(int irq, void (*handler)(void)) {
    if (irq < 16) {
        irq_handlers[irq] = handler;  
    }
}

void irq_handler_c(uint64_t *stack) {
    int irq = (int)stack[0];

    if (irq >= 0 && irq < 16 && irq_handlers[irq]) {
        irq_handlers[irq]();
    } else {
        kprint_error("Unhandled IRQ: ");
        kprint(irq);
    }

    pic_send_eoi(irq);
}