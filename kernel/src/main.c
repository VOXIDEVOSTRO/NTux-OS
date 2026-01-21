//basic limine includes
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>

//res
#include <kernel_res/images/background.h>
#include <libc/printf.h>

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
#include <drivers/fs/FAT/fat32.h>
#include <drivers/sata/ata.h>
#include <drivers/ps2/mose.h>

//mem includes
#include <mem/vmm.h>
#include <mem/pmm.h>
#include <mem/kmalloc.h>
#include <mem/umalloc.h>

//fs includes
#include <fs/ramfs.h>

//kernel lib includes
#include <libc/string.h>
#include <kernel_lib/info.h>
#include <kernel_lib/bool.h>

//test
#include <operators/power.h>

//elf loading
#include <drivers/elf/elf.h>

//cpu
#include <cpu/thread.h>

//debug

__attribute__((used, section(".limine_requests")))
static volatile struct limine_module_request module_request = {
    .id = LIMINE_MODULE_REQUEST_ID,
    .revision = 0
};

bool is_mouse_connected = false;
static Fat fat; 

static int BACKGROUND_WT;
static int BACKGROUND_HT;
static uint64_t background_shell;

volatile struct limine_framebuffer* front_buffer;
volatile struct limine_framebuffer* back_buffer;
static struct limine_memmap_response *memmap;
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

static volatile struct limine_module_response *module_response = NULL;
static DiskOps ops;

void init_kernel_lib(void) {}

void swap_buffers() {
    volatile struct limine_framebuffer* temp = front_buffer;
    front_buffer = back_buffer;
    back_buffer = temp;
}

void init_fb(void) {
    if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) {
        // Fehlerbehandlung, falls kein framebuffer vorhanden ist
    }

    back_buffer = framebuffer_request.response->framebuffers[0];
    fb_width = (int)back_buffer->width;
    fb_height = (int)back_buffer->height - 3;

    front_buffer = framebuffer_request.response->framebuffers[0]; // Initial Frontbuffer
    back_buffer = framebuffer_request.response->framebuffers[0];  // Initial Backbuffer

    BACKGROUND_HT = BACKGROUND_HEIGHT;
    BACKGROUND_WT = BACKGROUND_WIDTH;
    background_shell = background;

    clear_screen_lim(back_buffer, COLOR_BLACK);  

    shell_cursor = &shell_cursor_struct;
    init_cursor(shell_cursor, fb_width, fb_height);
    init_kprint_global(back_buffer, shell_cursor, color);  

    kprint_ok("Framebuffer init completed\n");
}

void init_ramfs_test() {
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

void fat_test(void) {
    kprint("Running FAT32 startup test...\n");

    DirInfo info;
    int err = fat_stat("/disk0", &info);

    if (err != FAT_ERR_NONE) {
        kprint_error("Cannot stat /disk0: ");
        kprint_error(fat_get_error(err));
        kprint("\n");
        return;
    }

    kprint_ok("/disk0 is accessible\n");

    kprint("Listing /disk0:\n");
    fat_ls("/disk0");

    kprint_ok("FAT32 startup test completed\n");
}


void init_fs(void) {
    if (!ata_init()) {
        kprint_error("No ATA device found\n");
        return;
    }

    kprint_ok("ATA device detected\n");

    uint64_t bytes = ata_get_total_space_bytes();
    uint64_t sectors = bytes / 512;

    kprint("Disk detected:\n");

    kprint(" - Size: ");
    kprint_uint(bytes / (1024 * 1024 * 1024));
    kprint(" GiB\n");

    kprint(" - Size (MB): ");
    kprint_uint(bytes / (1024 * 1024));
    kprint(" MB\n");

    kprint(" - Bytes: ");
    kprint_uint(bytes);
    kprint("\n");

    kprint(" - Sectors: ");
    kprint_uint(sectors);
    kprint("\n");

    kprint(" - Sector size: 512 bytes\n");

    DiskOps ops;
    if (!ata_get_disk_ops(&ops)) {
        kprint_error("Failed to load DiskOps\n");
        return;
    }

    uint8_t buf[512];
    if (!ops.read(buf, 0)) {
        kprint_error("Failed to read MBR (sector 0)\n");
        return;
    }

    if (buf[510] == 0x55 && buf[511] == 0xAA)
        kprint_ok("Valid MBR signature\n");
    else
        kprint_error("Invalid MBR signature\n");

    kprint("Probing for FAT32 filesystem...\n");

    int err = fat_probe(&ops, 0);
    if (err != FAT_ERR_NONE) {
        kprint_error("FAT32 probe failed: ");
        kprint_error(fat_get_error(err));
        kprint("\n");
        return;
    }

    kprint_ok("FAT32 filesystem detected\n");

    static Fat fat;
    err = fat_mount(&ops, 0, &fat, "disk0");
    if (err != FAT_ERR_NONE) {
        kprint_error("FAT32 mount failed: ");
        kprint_error(fat_get_error(err));
        kprint("\n");
        return;
    }

    kprint_ok("FAT32 mounted successfully at /disk0\n");

    fat_test();   
}


void init_mem(void) {
    kprint_ok("initing mem ");
    memmap = memmap_request.response;
    vmm_init(memmap);
    kprint_ok("mem init completed ");
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

    if (interrupts_are_enabled()) {
        kprint_ok("interrupts are working and enabled");
    } else {
        shell_cursor = &shell_cursor_struct;
        init_cursor(shell_cursor, fb_width, fb_height);
        init_kprint_global(back_buffer, shell_cursor, color);
        kprint_error("interrupts are not enabled");
        clear_screen_lim(back_buffer, COLOR_LIGHT_BLUE_SCREEN_BG);
        kprint(":(\n\n");
        kprint_error("Critical Error: Interrupts failed to enable.\nSystem Halted.");
        __asm__ volatile ("hlt");
    }
}

void init_cpu() {
    thread_init();
}

void init_drivers(void) {
    keyboard_init();
    kprint_ok("PS/2 keyboard found and initialized");
    bool connected = mouse_init();
    is_mouse_connected = connected;
    kprint_ok("PS/2 Mouse found and initialized");
    kprint_ok("PCI bus initialized");
    beep(440, 5);
    kprint_ok("Audio initialized");
}

void thread1_function(void) {
    while (1) {
        kprint("message from thread 1\n");
        thread_yield();
    }
}

void thread2_function(void) {
    while (1) {
        kprint("message from thread 2\n");
        thread_yield();
    }
}

void init_gpu(void){
    
}

void init_kernel(void) {
    init_fb();
    init_interrupts();
    init_drivers();
    init_mem();
    init_ramfs_test(); 
    //init_acpi();
    //kprint_ok("init ACPI");
    init_cpu();
    init_fs();
    init_gpu();
    boolean running = true;
    kprint_ok(boolean_to_string(running));
    kprint("\n");
    running = false;
    kprint_ok(boolean_to_string(running));
    kprint("\n");
    int second = 100;
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

    back_buffer = framebuffer_request.response->framebuffers[0];
    fb_width = (int)back_buffer->width;
    fb_height = (int)back_buffer->height - 3;

    clear_screen_lim(back_buffer, COLOR_BLACK); 

    shell_cursor = &shell_cursor_struct;
    init_cursor(shell_cursor, fb_width, fb_height);
    init_kprint_global(back_buffer, shell_cursor, color);  
    draw_image_from_uint64_t(back_buffer, background_shell, BACKGROUND_WT, BACKGROUND_HT);

    swap_buffers();  
}

static void shell_backspace() {  
    if (input_len == 0) return;

    input_len--;
    input_buffer[input_len] = '\0';

    if (shell_cursor->x >= shell_cursor->char_width) {
        shell_cursor->x -= shell_cursor->char_width;
    } else if (shell_cursor->y > 0) {
        shell_cursor->y -= shell_cursor->char_height;
        shell_cursor->x = fb_width - shell_cursor->char_width;
    }

    draw_char_lim(back_buffer, fb_width, shell_cursor->x, shell_cursor->y, COLOR_BLACK);  
    cursor_visible = 1;
    render_cursor_lim(back_buffer, shell_cursor, color);
}

static void shell_print_prompt() {
    const char* prompt = "<NTux-OS> :";  
    for (int i = 0; prompt[i]; i++)
        put_char_with_cursor_lim(back_buffer, shell_cursor, prompt[i], color);  
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
    } else if (strcmp(cmd, "Hello World!") == 0) {
        kprint("Bro why did you type this ??????\n");
    } else if (strcmp(cmd, "test") == 0) {
        play_slainewin_easteregg();
    } else if (strcmp(cmd, "setbg1") == 0) {
        BACKGROUND_WT = BACKGROUND_WIDTH;
        BACKGROUND_HT = BACKGROUND_HEIGHT;
        background_shell  = background;
        shell_clear_screen();
        kprint("seted background 1\n");
    } else if (strcmp(cmd, "pcilist") == 0) {
    } else if (strcmp(cmd, "thread_test") == 0) {
        thread_create(thread1_function);
        thread_create(thread2_function);
        thread_yield();
    } else if (strcmp(cmd, "cpuinfo") == 0) {
        info_cmd_cpuinfo();
    } else if (strcmp(cmd, "ls") == 0) {
        fat_ls("/");
    }else if (strcmp(cmd, "rls") == 0) {
        ramfs_list_dir("/");  
    }else if (strncmp(cmd, "rmkdir ", 7) == 0) {        
    const char* path = cmd + 7;                   
    while (*path == ' ') path++;
    
    if (strlen(path) == 0) {
        kprint("rmkdir: missing directory name\n");
    } else {
        int result = ramfs_mkdir(path);  
        if (result == 0) {
            kprint("Directory created: ");
            kprint(path);
            kprint("\n");
        } else {
            kprint("rmkdir failed ");
            kprint("\n");
        }
    }
} else if (strcmp(cmd, "uptime") == 0) {
        info_cmd_uptime();
    }  else if (strcmp(cmd, "version") == 0) {
        kprint("version 0.0.1 build 15\n");
    } else if (strncmp(cmd, "echo ", 5) == 0) {
        kprint(cmd + 5); 
        kprint("\n");
    } else {
        if (strlen(cmd) == 0) return;
        kprint("Unknown command: ");
        kprint(cmd);  
        kprint("\n");
    }
}

static void shell_handle_key(char c) {
    if (cursor_visible) clear_cursor_lim(back_buffer, shell_cursor);
    if (c == '\n') {
        input_buffer[input_len] = '\0';  
        kprint("\n");
        shell_execute_command(input_buffer);
        shell_clear_input();  
        shell_print_prompt();  
    }  else if (c == '\b') {
        if (input_len > 0) {
            input_len--;
            input_buffer[input_len] = '\0';
            shell_backspace();
        }
    } else if (c >= 32 && c < 127) {  
        if (input_len < SHELL_MAX_INPUT - 1) {
            input_buffer[input_len++] = c;  
            put_char_with_cursor_lim(back_buffer, shell_cursor, c, color);  
        }
    }
    cursor_visible = 1;
    render_cursor_lim(back_buffer, shell_cursor, color);
    swap_buffers();
}

int last_blink_tick = 0;

void update_cursor_blink(void) {
    if (get_tick_count() - last_blink_tick >= 20) {
        last_blink_tick = get_tick_count();

        if (cursor_visible) {
            clear_cursor_lim(back_buffer, shell_cursor);
            cursor_visible = false;
        } else {
            render_cursor_lim(back_buffer, shell_cursor, color);
            cursor_visible = true;
        }
        last_blink_tick = get_tick_count();
    }
}

void load_user_space(void) {
    if (!module_response || module_response->module_count == 0) {
        kprint("[USERSPACE] Kein init.elf als Modul geladen → bleibe im Kernel\n");
        return;
    }

    void *init_elf = (void*)module_response->modules[0]->address;

    kprint_ok("[USERSPACE] Lade: ");

    void *entry = elf64_load_and_prepare(init_elf);
    if (!entry) {
        kprint_error("[USERSPACE] ELF fehlerhaft oder kaputt!\n");
        return;
    }

    kprint_ok("[USERSPACE] Userspace bereit ");
    kprint("Wechsel in Ring 3...\n");
    //sleep_s(2);

    //elf64_enter_ring3((uint64_t)entry);
}

void kmain(void) {
    module_response = module_request.response;
    last_blink_tick = get_tick_count();
    init_kernel();
    shell_clear_screen();
    kprint("Welcome to NTux-OS!\n");
    load_user_space();
    sleep_s(1);
    shell_print_prompt();
    update_cursor_blink();
    while (1) {
        keyboard_poll();  
        char c;
        if (keyboard_getchar(&c)) {
            shell_handle_key(c);
        }
        update_cursor_blink();  
        swap_buffers();  
        __asm__ volatile("hlt");
    }
}
