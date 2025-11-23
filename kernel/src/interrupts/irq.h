#ifndef IRQ_H
#define IRQ_H

#include <stdint.h>


void irq_register_handler(int irq, void (*handler)(void));
void irq_handler_c(uint64_t *stack);

#endif
