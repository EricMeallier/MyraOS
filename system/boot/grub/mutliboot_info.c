#include <stdint.h>
#include <stdbool.h>

#include "vmm/vmm.h"

#define MB_TAG_TYPE_FRAMEBUFFER 8

uint32_t multiboot_info_addr;

typedef struct multiboot2_tag_framebuffer_t {
    uint32_t type;
    uint32_t size;
    uint64_t framebuffer_addr;
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint8_t  framebuffer_bpp;
    uint8_t  framebuffer_type;
    uint16_t reserved;
} __attribute__((packed)) multiboot2_tag_framebuffer_t;

void parse_multiboot_info(uint32_t addr) {
    addr += 8; // skip total size + reserved

    while (1) {
        uint32_t type = *(uint32_t*) addr;
        uint32_t size = *(uint32_t*)(addr + 4);

        if (type == 0) {
            break;
        }

        if (type == MB_TAG_TYPE_FRAMEBUFFER) {
            multiboot2_tag_framebuffer_t* fb = (multiboot2_tag_framebuffer_t*) addr;

            uint32_t fb_phys = (uint32_t)(fb->framebuffer_addr);
            uint32_t fb_size = fb->framebuffer_pitch * fb->framebuffer_height;

            for (uint32_t i = 0; i < fb_size; i += PAGE_SIZE) {
                vmm_map_page(fb_phys + i, fb_phys + i, PAGE_PRESENT | PAGE_WRITE);
            }

            volatile uint32_t* framebuffer = (volatile uint32_t*) fb_phys;
            for (uint32_t y = 0; y < fb->framebuffer_height; y++) {
                for (uint32_t x = 0; x < fb->framebuffer_width; x++) {
                    framebuffer[y * (fb->framebuffer_pitch / sizeof(uint32_t)) + x] = 0xFF | ((x * 255 / fb->framebuffer_width) << 16) | ((y * 255 / fb->framebuffer_height));
                }
            }
        }

        addr += (size + 7) & ~7; // 8-byte align
    }
}
