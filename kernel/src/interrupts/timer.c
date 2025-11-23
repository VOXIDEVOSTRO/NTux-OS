#include <interrupts/irq.h>
#include <drivers/framebuffer/kprint.h>
#include <interrupts/pic.h>
#include "timer.h"


static uint64_t tick_count = 0;

void timer_handler(void) {
    tick_count++;

    if (tick_count % 100 == 0) {
        kprint("Timer ticked 100 times!\n");
    }
    pic_send_eoi(0);

}
void init_timer() {
    timer_pit_config();
    irq_register_handler(0, timer_handler);
    kprint_ok("Timer initialized");
}

void sleep(uint32_t ticks) {
    uint64_t start_tick = tick_count;
    while (tick_count - start_tick < ticks) {
        __asm__ volatile("hlt");  
    }
}

void sleep_s(uint32_t seconds) {
    uint32_t ticks_per_second = 100; 
    sleep(seconds * ticks_per_second);
}

void sleep_m(uint32_t minutes) {
    uint32_t ticks_per_minute = 60 * 100;  
    sleep(minutes * ticks_per_minute);
}