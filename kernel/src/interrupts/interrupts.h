#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <stdint.h>

void interrupts_enable();
void interrupts_disable();
int interrupts_are_enabled();

#endif
