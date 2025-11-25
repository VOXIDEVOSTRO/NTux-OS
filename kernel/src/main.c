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
#include <interrupts/irq.h> 
#include <interrupts/timer.h> 

//driver includes
#include <drivers/ps2/keyboard.h>

//mem includes
#include <mem/vmm.h>
#include <mem/pmm.h>
#include <mem/kmalloc.h>
#include <mem/umalloc.h>
//fs includes
#include <fs/ramfs.h>


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
__attribute__((used, section(".limine_requests")))
volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST_ID,
    .revision = 0
};


void init_kernel_lib(void){
    
}

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

void init_ramfs_test(){
    kprint_ok("initing ramfs");
    ramfs_init();

    kprint("=== ROOT TEST ===\n");


    kprint("=== MKDIR /etc ===\n");
    ramfs_mkdir("/etc");

    kprint("=== LIST ROOT ===\n");
    ramfs_list_dir("/");
    
    kprint("=== CREATE FILE ===\n");
    ramfs_create_file("/etc/hosts", "127.0.0.1 localhost");

    kprint("=== LIST /etc ===\n");
    ramfs_list_dir("/etc");

    kprint("=== READ FILE ===\n");
    ramfs_read_file("/etc/hosts");
    ramfs_delete_file("/etc/hosts");
    ramfs_delete_file("/");


    kprint("=== LIST /etc AFTER DELETE ===\n");
    ramfs_list_dir("/");

}
void init_mem(void){
    kprint_ok("initing mem ");
    struct limine_memmap_response *memmap = memmap_request.response;
    vmm_init(memmap);
    kprint_ok("mem init completet ");
}


void init_interrupts(void) {
    interrupts_disable();
    pic_init();
    kprint_ok("PIC init");
    gdt_init();
    kprint_ok("GDT init");
    idt_init();
    kprint_ok("IDT init");
    init_timer();
    kprint_ok("Timer init");
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
        clear_screen_lim(framebuffer, COLOR_LIGHT_BLUE_SCREEN_BG);
        kprint(":(\n\n");
        kprint_error("Critical Error: Interrupts failed to enable.\nSystem Halted.");
        __asm__ volatile ("hlt");
    }
    
}

void init_drivers(void) {
    ps2_init();
    kprint_ok("PS/2 controller initialized");
}

void init_kernel(void) {
    init_fb();
    init_interrupts();
    init_drivers();
    init_mem();
    init_ramfs_test();
    for (int i = 0; i < 900; i++){    
        sleep_s(1);
        kprint_ok("1 second passed");
    }
    kprint_ok("Kernel initialized.");
}



void kmain(void) {
    init_kernel();
    kprint("Welcome to NTux-OS!\n");
    while (1) {
        uint8_t sc = ps2_read_data();
        uint16_t key = sct1[sc];
        uint16_t special_key = detect_special_key(sc);
        if (key < 0x0100) {
            char c = (char)key;
            kprintcolor(&c, COLOR_LIGHT_GREEN);
        } 
        __asm__ volatile("hlt");
    }
}
