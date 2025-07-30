#include "gfx/gfx.h"

#include <stddef.h>

#include "heap/heap.h"
#include "libc_kernel/string.h"

static argb_t* double_buffer;

void gfx_init(void) {
    uint32_t buffer_size = fb_info.pitch * fb_info.height;
    double_buffer = kmalloc(buffer_size);
    kmemset(double_buffer, 0, buffer_size);
}

void gfx_draw_pixel(uint32_t x, uint32_t y, argb_t color) {
    if (x >= fb_info.width || y >= fb_info.height) { 
        return;
    }

    uint32_t offset = y * fb_info.width + x;
    double_buffer[offset] = color;
}

void gfx_draw_line(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, argb_t color) {
    int dx = x1 > x0 ? x1 - x0 : x0 - x1;
    int dy = y1 > y0 ? y1 - y0 : y0 - y1;
    dy = -dy;
    
    int sx = x1 > x0 ? 1 : -1;
    int sy = y1 > y0 ? 1 : -1;

    int err = dx + dy;

    while (true) {
        gfx_draw_pixel(x0, y0, color);

        if (x0 == x1 && y0 == y1) {
            break;
        }

        int err2 = 2 * err;
        if (err2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if (err2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void gfx_draw_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, argb_t color) {
    uint32_t x1 = x + width;
    uint32_t y1 = y + height;

    gfx_draw_line(x, y, x1, y, color);
    gfx_draw_line(x, y, x, y1, color);
    gfx_draw_line(x, y1, x1, y1, color);
    gfx_draw_line(x1, y, x1, y1, color);
}

void gfx_fill_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, argb_t color) {
    for (uint32_t cy = y; cy < y + height; cy++) {
        gfx_draw_line(x, cy, x + width, cy, color);
    }
}

void gfx_clear(argb_t color) {
    for (uint32_t y = 0; y < fb_info.height; y++) {
        for (uint32_t x = 0; x < fb_info.width; x++) {
            gfx_draw_pixel(x, y, color);
        }
    }
}

void gfx_flush(void) {
    for (uint32_t y = 0; y < fb_info.height; y++) {
        for (uint32_t x = 0; x < fb_info.width; x++) {
            uint32_t offset = y * fb_info.width + x;
            fb_draw_pixel(x, y, double_buffer[offset]);
        }
    }
}