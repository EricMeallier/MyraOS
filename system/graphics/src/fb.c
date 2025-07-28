#include "fb/fb.h"

fb_info_t fb_info;
static argb_t* fb_addr;

void fb_init(fb_info_t info) {
    fb_info = info;
    fb_addr = (argb_t*) info.addr;
}

void fb_put_pixel(uint32_t x, uint32_t y, argb_t color) {
    if (x >= fb_info.width || y >= fb_info.height) {
        return;
    }

    uint32_t offset = y * (fb_info.pitch / sizeof(argb_t)) + x;
    fb_addr[offset] = color;
}

argb_t fb_get_pixel(uint32_t x, uint32_t y) {
    if (x >= fb_info.width || y >= fb_info.height) {
        return 0;
    }

    uint32_t offset = y * (fb_info.pitch / sizeof(argb_t)) + x;
    return fb_addr[offset];
}