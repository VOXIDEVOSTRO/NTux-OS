//basic limine includes
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>

//res
#include <kernel_res/images/background.h>


//interrupts includes
#include <interrupts/gdt.h>
#include <interrupts/idt.h>
#include <interrupts/pic.h>
#include <interrupts/interrupts.h>
#include <interrupts/irq.h> 
#include <interrupts/timer.h> 

//driver includes
#include <drivers/ps2/keyboard.h>
#include <drivers/pci/pci.h>
#include <drivers/framebuffer/fb.h>
#include <drivers/framebuffer/kprint.h>
#include <drivers/audio/audio.h>

//mem includes
#include <mem/vmm.h>
#include <mem/pmm.h>
#include <mem/kmalloc.h>
#include <mem/umalloc.h>


//fs includes
#include <fs/ramfs.h>

//kernel lib includes
#include <kernel_lib/string.h>
#include <kernel_lib/info.h>
#include <kernel_lib/bool.h>

//test
#include <operators/power.h>



static volatile struct limine_framebuffer* framebuffer;
static struct limine_memmap_response *memmap ;
static int fb_width, fb_height;
static cursor_t shell_cursor_struct;
cursor_t* shell_cursor;
uint32_t color = COLOR_WHITE;

struct limine_shutdown_request {
    uint64_t id[4];
    uint64_t revision;
    struct limine_shutdown_response *response;
    void (*callback)(void);
};

struct limine_shutdown_response {
    uint64_t revision;
};


#ifndef LIMINE_SHUTDOWN_REQUEST
#define LIMINE_SHUTDOWN_REQUEST 0x8c6c3b258c6c3b25ULL, 0x8c6c3b258c6c3b25ULL, \
                                0x8c6c3b258c6c3b25ULL, 0x8c6c3b258c6c3b25ULL
#endif

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




__attribute__((used, section(".limine_requests")))
static volatile struct limine_shutdown_request shutdown_request = {
    .id = { LIMINE_SHUTDOWN_REQUEST },
    .revision = 0
};

void (*limine_shutdown_callback)(void) = NULL;

void init_kernel_lib(void){
    
}

void init_fb(void) {
    if (framebuffer_request.response == NULL ||
        framebuffer_request.response->framebuffer_count < 1) {
    }

    framebuffer = framebuffer_request.response->framebuffers[0];
    fb_width = (int)framebuffer->width;
    fb_height = (int)framebuffer->height-3;

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
    ramfs_create_file("readme.txt", "This is a test file in RAMFS.");

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
    memmap= memmap_request.response;
    vmm_init(memmap);
    kprint_ok("mem init completet ");
}


void init_interrupts(void) {
    interrupts_disable();
    gdt_init();
    kprint_ok("GDT init");
    pic_init();
    kprint_ok("PIC init");
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
    keyboard_init();
    kprint_ok("PS/2 controller initialized");
    pci_init();
    kprint_ok("PCI bus initialized");
    beep(440, 5);
    kprint_ok("Audio initzialized");
}


void init_kernel(void) {
    init_fb();
    init_interrupts();
    init_drivers();
    init_mem();
    init_ramfs_test();
    boolean running = true;
    kprint_ok(boolean_to_string(running));
    kprint("\n");
    running = false;
    kprint_ok(boolean_to_string(running));
    kprint("\n");
    int second = 3;
    kprint("Sleeping for 3 seconds...\n");
    sleep_s(second);
    kprint_ok("Kernel initialized.");
    play_startup_sound();
}


/* test shell here*/
#define SHELL_MAX_INPUT 128
#define CURSOR_BLINK_LOOPS 8000

static char input_buffer[SHELL_MAX_INPUT];  
static int input_len = 0;  

int cursor_visible = 1;  
int blink_counter = 0;  

// clear method
static void shell_clear_screen() {
    if (framebuffer_request.response == NULL ||
        framebuffer_request.response->framebuffer_count < 1) {
    }

    framebuffer = framebuffer_request.response->framebuffers[0];
    fb_width = (int)framebuffer->width;
    fb_height = (int)framebuffer->height-3;

    clear_screen_lim(framebuffer, COLOR_BLACK);

    shell_cursor = &shell_cursor_struct;
    init_cursor(shell_cursor, fb_width, fb_height);
    init_kprint_global(framebuffer, shell_cursor, color);
}

static void shell_backspace() {
    if (input_len > 0) {
        input_len--;  
        input_buffer[input_len] = '\0';  

        if (cursor_visible) clear_cursor_lim(framebuffer, shell_cursor);  
        if (shell_cursor->x >= shell_cursor->char_width) {
            shell_cursor->x -= shell_cursor->char_width;  
        } else if (shell_cursor->y > 0) {
            shell_cursor->y -= shell_cursor->char_height;  
            shell_cursor->x = fb_width - shell_cursor->char_width;  
        }

        draw_char_lim(framebuffer->address, fb_width, shell_cursor->x, shell_cursor->y, 0x000000);  

        cursor_visible = 1;
        render_cursor_lim(framebuffer, shell_cursor, color);  
    }
}

static void shell_print_prompt() {
    const char* prompt = "<NTux-OS> :";  
    for (int i = 0; prompt[i]; i++)
        put_char_with_cursor_lim(framebuffer, shell_cursor, prompt[i], color);  
}

static void shell_clear_input() {
    input_len = 0;
    input_buffer[0] = '\0';  
}


static void shell_execute_command(const char* cmd) {
    if (strcmp(cmd, "help") == 0) {
        kprint("Commands:\n  help     - show this help\n  clear    - clear screen\n  reboot   - reboot the system\n    version  - shows os version\n   echo     - prints what ever you want \n");
    } else if (strcmp(cmd, "clear") == 0) {
        shell_clear_screen();  
    } else if (strcmp(cmd, "reboot") == 0) {
        kprint("Rebooting in 3 seconds...\n");
        sleep_s(1);  
        kprint("Rebooting in 2 seconds...\n");
        sleep_s(1);  
        kprint("Rebooting in 1 second...\n");
        sleep_s(1);  
        power_reboot();
    }else if (strcmp(cmd, "Hello World!") == 0){
        kprint("Bro why did you type this ??????\n");
    }else if (strcmp(cmd, "test") == 0){
        play_slainewin_easteregg();
    }else if (strcmp(cmd, "shutdown") == 0) {
        kprint("Powering off in 3 seconds...\n");
        sleep_s(1);  
        kprint("Powering off in 2 seconds...\n");
        sleep_s(1);  
        kprint("Powering off in 1 second...\n");
        sleep_s(1);  
        power_shutdown();
    }else if (strcmp(cmd, "cpuinfo") == 0) {
        info_cmd_cpuinfo();
    }
    else if (strcmp(cmd, "meminfo") == 0) {
        info_cmd_meminfo();
    }
    else if (strcmp(cmd, "uptime") == 0) {
        info_cmd_uptime();
    }  else if (strcmp(cmd, "version") == 0) {
        kprint("version 0.0.1 build 15\n");
    } else if (strncmp(cmd, "echo ", 5) == 0) {
        kprint(cmd + 5); 
        kprint("\n");
    }else if (strncmp(cmd, "rmkdir ", 7) == 0) {        
        const char* path = cmd + 7;                   
        while (*path == ' ') path++;
    
        if (strlen(path) == 0) {
            kprint("rmkdir: missing directory name\n");
        } else {
            int result = ramfs_mkdir(path);
            kprint("Directory created: ");
            kprint(path);
            kprint("\n");
        }
    }else if (strcmp(cmd, "rls") == 0) {
        ramfs_list_dir("/");
    }else if (strncmp(cmd, "read ", 5) == 0) {
    const char* path = cmd + 5;
    while (*path == ' ' || *path == '\t') path++;  

    if (*path == '\0') {
        kprintcolor("read: missing file path!\n",COLOR_LIGHT_RED );
        kprint("Usage: read /readme.txt\n");
        return;
    }

    const char * output=ramfs_read_file(path);
    kprint(output);
}
    else if (strlen(cmd) > 0) {
        kprint("Unknown command: ");
        kprint(cmd);  
        kprint("\n");
    }
}

static void shell_handle_key(char c) {
    if (cursor_visible) clear_cursor_lim(framebuffer, shell_cursor);

    if (c == '\n') {  
        input_buffer[input_len] = '\0';  
        kprint("\n");
        shell_execute_command(input_buffer);
        shell_clear_input();  
        shell_print_prompt();  
    } else if (c == '\b') {  
        shell_backspace();  
    } else if (c >= 32 && c < 127) {  
        if (input_len < SHELL_MAX_INPUT - 1) {
            input_buffer[input_len++] = c;  
            put_char_with_cursor_lim(framebuffer, shell_cursor, c, color);  
        }
    }
    cursor_visible = 1;
    render_cursor_lim(framebuffer, shell_cursor, color);
}

int last_blink_tick = 0;

static void update_cursor_blink(void) {
    if (get_tick_count() - last_blink_tick >= 20) {
        last_blink_tick = get_tick_count();

        if (cursor_visible) {
            clear_cursor_lim(framebuffer, shell_cursor);
            cursor_visible = false;
        } else {
            render_cursor_lim(framebuffer, shell_cursor, color);
            cursor_visible = true;
        }
        last_blink_tick = get_tick_count();
    }
}


//kernel main function

void kmain(void) {
    last_blink_tick = get_tick_count();
    init_kernel();
    shell_clear_screen();
    draw_image_from_uint64_t(framebuffer,background,BACKGROUND_WIDTH,BACKGROUND_HEIGHT);
    kprint("Welcome to NTux-OS!\n");
    shell_print_prompt();
    update_cursor_blink();
    while (1) {
        keyboard_poll();  
        char c;
        if(keyboard_getchar(&c)){
             shell_handle_key(c);
        }
        update_cursor_blink();  
        __asm__ volatile("hlt");
    }
}