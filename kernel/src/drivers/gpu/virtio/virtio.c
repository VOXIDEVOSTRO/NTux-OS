#include "virtio.h"
#include <kernel_res/fonts/font8x8/font8x8_basic.h>
#include <drivers/framebuffer/kprint.h>
#include <kernel_lib/vector.h>
#include <kernel_lib/io.h>


static virtio_gpu_t virtio_gpu;

int abs(int x) {
    return (x < 0) ? -x : x;
}


void virtio_gpu_init(pci_device_t *device) {
    pci_bar_t bar;
    pci_get_bar(&bar, device->bus, 0);  
    virtio_gpu.base_address = (volatile uint32_t*) bar.u.address;
    virtio_gpu.width = 1920;   
    virtio_gpu.height = 1080;
    virtio_gpu.stride = virtio_gpu.width * 4;  
    kprintf("VirtIO GPU initialized with resolution %dx%d.\n", virtio_gpu.width, virtio_gpu.height);
}

void virtio_gpu_draw_pixel(uint32_t x, uint32_t y, uint32_t color) {
    if (x >= virtio_gpu.width || y >= virtio_gpu.height) return; 

    volatile uint32_t *pixel = virtio_gpu.base_address + (y * virtio_gpu.stride / 4) + x;
    *pixel = color;
}

void virtio_gpu_draw_line(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t color) {
    int dx = x2 - x1;
    int dy = y2 - y1;
    
    int steps = (abs(dx) > abs(dy)) ? abs(dx) : abs(dy);
    int x_increment = dx * 256 / steps;
    int y_increment = dy * 256 / steps;

    int x = x1 * 256;
    int y = y1 * 256;

    for (int i = 0; i <= steps; i++) {
        virtio_gpu_draw_pixel(x / 256, y / 256, color);
        x += x_increment;  
        y += y_increment;  
    }
}


void virtio_gpu_draw_text(uint32_t x, uint32_t y, const char *text, uint32_t color) {
    while (*text) {
        uint8_t ch = *text;
        if (ch < 32 || ch > 126) {
            text++;
            continue;  
        }

        for (int i = 0; i < 9; i++) {
            uint8_t line = font8x8_basic[ch][i];
            for (int j = 0; j < 9; j++) {
                if (line & (1 << (8 - j))) {
                    virtio_gpu_draw_pixel(x + j, y + i, color);
                }
            }
        }

        text++;
        x += 9; 
    }
}
