#ifndef VIRTIO_GPU_H
#define VIRTIO_GPU_H

#include <stdint.h>
#include <kernel_lib/io.h>
#include <drivers/pci/pci.h>

#define MAX_WIDTH      1920  
#define MAX_HEIGHT     1080

#define VIRTIO_VENDOR_ID 0x1AF4 // Virtio Vendor ID
#define VIRTIO_GPU_DEVICE_ID 0x1050 // Virtio GPU Device ID


typedef struct {
    volatile uint32_t *base_address; 
    uint32_t width;                  
    uint32_t height;                 
    uint32_t stride;                 
} virtio_gpu_t;


void virtio_gpu_init(pci_device_t *device); 
void virtio_gpu_draw_pixel(uint32_t x, uint32_t y, uint32_t color); 
void virtio_gpu_draw_line(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t color); 
void virtio_gpu_draw_text(uint32_t x, uint32_t y, const char *text, uint32_t color); 


#endif 
