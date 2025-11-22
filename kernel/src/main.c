#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>

// kernel includes
#include <drivers/framebuffer/fb.h>
#include <drivers/framebuffer/kprint.h>

//interrupts includes
#include <interrupts/gdt.h>
#include <interrupts/idt.h>
#include <interrupts/pic.h>
#include <interrupts/interrupts.h>

static volatile struct limine_framebuffer* framebuffer;
static int fb_width, fb_height;
static cursor_t shell_cursor_struct;
cursor_t* shell_cursor;
uint32_t color = COLOR_WHITE;

__attribute__((used, section(".limine_requests")))
volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = 0
};

void init_fb(void) {
    if (framebuffer_request.response == NULL ||
        framebuffer_request.response->framebuffer_count < 1) {
    }

    framebuffer = framebuffer_request.response->framebuffers[0];
    fb_width = (int)framebuffer->width;
    fb_height = (int)framebuffer->height;

    clear_screen_lim(framebuffer, COLOR_BLACK);

    shell_cursor = &shell_cursor_struct;
    init_cursor(shell_cursor, fb_width, fb_height);
    init_kprint_global(framebuffer, shell_cursor, color);

    kprint_ok("frambuffer init");
}

void init_interrupts(void) {
    interrupts_disable();
    pic_init();
    kprint_ok("PIC init");
    gdt_init();
    kprint_ok("GDT init");
    idt_init();
    kprint_ok("IDT init");

    interrupts_enable();
    kprint_ok("Enabled interrupts");

    if (interrupts_are_enabled())
    {
        kprint_ok("interrupts are working and enabled");
    }else if(!interrupts_are_enabled()){
        shell_cursor = &shell_cursor_struct;
        init_cursor(shell_cursor, fb_width, fb_height);
        init_kprint_global(framebuffer, shell_cursor, color);
        kprint_error("interrupts are not enabled");
        clear_screen_lim(framebuffer, COLOR_BLUE);
        kprint(":(\n\n");
        kprint_error("Critical Error: Interrupts failed to enable.\nSystem Halted.");
        __asm__ volatile ("hlt");
    }
    

}

void init_kernel(void) {
    init_fb();
    init_interrupts();
    kprint_ok("Kernel initialized.");
}

void kmain(void) {
    init_kernel();

    for(;;){
        __asm__ volatile ("hlt");
    }
}
