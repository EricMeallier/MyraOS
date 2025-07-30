#include "gfx/gfx.h"

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

void gfx_draw_circle(uint32_t cx, uint32_t cy, uint32_t radius, argb_t color) {
    int x = radius;
    int y = 0;
    int err = 0;

    while (x >= y) {
        gfx_draw_pixel(cx + x, cy + y, color);
        gfx_draw_pixel(cx + y, cy + x, color);
        gfx_draw_pixel(cx - y, cy + x, color);
        gfx_draw_pixel(cx - x, cy + y, color);
        gfx_draw_pixel(cx - x, cy - y, color);
        gfx_draw_pixel(cx - y, cy - x, color);
        gfx_draw_pixel(cx + y, cy - x, color);
        gfx_draw_pixel(cx + x, cy - y, color);

        y++;
        err += 1 + 2*y;
        if (2*(err - x) + 1 > 0) {
            x--;
            err += 1 - 2*x;
        }
    }
}

void gfx_fill_circle(uint32_t cx, uint32_t cy, uint32_t radius, argb_t color) {
    for (int y = -radius; y <= (int)radius; y++) {
        for (int x = -radius; x <= (int)radius; x++) {
            if (x * x + y * y <= (int)(radius * radius)) {
                gfx_draw_pixel(cx + x, cy + y, color);
            }
        }
    }
}

void gfx_draw_polygon(const uint32_t* xs, const uint32_t* ys, size_t count, argb_t color) {
    uint32_t lx = xs[0];
    uint32_t ly = ys[0];

    for (size_t i = 1; i < count; i++) {
        gfx_draw_line(lx, ly, xs[i], ys[i], color);

        lx = xs[i];
        ly = ys[i];
    }

    gfx_draw_line(lx, ly, xs[0], ys[0], color);
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