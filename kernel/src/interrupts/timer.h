#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

void timer_pit_config(void);
void init_timer(void);
void sleep(uint32_t ticks);
void sleep_s(uint32_t seconds);
void sleep_m(uint32_t minutes);
int get_tick_count();


#endif // TIMER_H
