#include "gfx/gfx.h"

#include "heap/heap.h"
#include "libc_kernel/string.h"

#define MAX_DIRTY_RECTS 32

typedef struct {
    int x_min, y_min, x_max, y_max;
    bool dirty;
} gfx_dirty_rect_t;

static gfx_dirty_rect_t dirty_rects[LAYER_COUNT][MAX_DIRTY_RECTS];

static inline argb_t blend_argb(argb_t bottom, argb_t top);
static void blend_pixel_layer(layer_id_t layer, uint32_t x, uint32_t y, argb_t color, uint8_t alpha);

static argb_t* double_buffer;
static argb_t* layers[LAYER_COUNT];

void gfx_init(void) {
    uint32_t buffer_size = fb_info.pitch * fb_info.height;

    double_buffer = kmalloc(buffer_size);
    kmemset(double_buffer, 0, buffer_size);

    for (uint32_t i = 0; i < LAYER_COUNT; i++) {
        layers[i] = kmalloc(buffer_size);
        kmemset(layers[i], 0, buffer_size);
    }

    for (uint32_t layer = 0; layer < LAYER_COUNT; layer++) {
        for (uint32_t i = 0; i < MAX_DIRTY_RECTS; i++) {
            dirty_rects[layer][i].dirty = false;
        }
    }
}

void gfx_draw_pixel(layer_id_t layer, uint32_t x, uint32_t y, argb_t color) {
    if (x >= fb_info.width || y >= fb_info.height) { 
        return;
    }

    uint32_t offset = y * fb_info.pixels_per_row + x;
    layers[layer][offset] = color;

    gfx_mark_dirty(layer, x, y);
}

argb_t gfx_get_pixel(uint32_t x, uint32_t y) {
    if (x >= fb_info.width || y >= fb_info.height) {
        return 0;
    }

    uint32_t offset = y * fb_info.pixels_per_row + x;
    return double_buffer[offset];
}

void gfx_draw_line(layer_id_t layer, uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, argb_t color) {
    int dx = x1 > x0 ? x1 - x0 : x0 - x1;
    int dy = y1 > y0 ? y1 - y0 : y0 - y1;
    dy = -dy;
    
    int sx = x1 > x0 ? 1 : -1;
    int sy = y1 > y0 ? 1 : -1;

    int err = dx + dy;

    while (true) {
        gfx_draw_pixel(layer, x0, y0, color);

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

void gfx_draw_rect(layer_id_t layer, uint32_t x, uint32_t y, uint32_t width, uint32_t height, argb_t color) {
    uint32_t x1 = x + width;
    uint32_t y1 = y + height;

    gfx_draw_line(layer, x, y, x1, y, color);
    gfx_draw_line(layer, x, y, x, y1, color);
    gfx_draw_line(layer, x, y1, x1, y1, color);
    gfx_draw_line(layer, x1, y, x1, y1, color);
}

void gfx_fill_rect(layer_id_t layer, uint32_t x, uint32_t y, uint32_t width, uint32_t height, argb_t color) {
    uint8_t alpha = color >> 24;

    if (alpha == 0xFF) {
        for (uint32_t cy = y; cy < y + height; cy++) {
            gfx_draw_line(layer, x, cy, x + width, cy, color);
        }
    } else {
        for (uint32_t cy = y; cy < y + height; cy++) {
            for (uint32_t cx = x; cx < x + width; cx++) {
                blend_pixel_layer(layer, cx, cy, color, alpha);
            }
        }
    }
}

void gfx_draw_circle(layer_id_t layer, uint32_t cx, uint32_t cy, uint32_t radius, argb_t color) {
    int x = radius;
    int y = 0;
    int err = 0;

    while (x >= y) {
        gfx_draw_pixel(layer, cx + x, cy + y, color);
        gfx_draw_pixel(layer, cx + y, cy + x, color);
        gfx_draw_pixel(layer, cx - y, cy + x, color);
        gfx_draw_pixel(layer, cx - x, cy + y, color);
        gfx_draw_pixel(layer, cx - x, cy - y, color);
        gfx_draw_pixel(layer, cx - y, cy - x, color);
        gfx_draw_pixel(layer, cx + y, cy - x, color);
        gfx_draw_pixel(layer, cx + x, cy - y, color);

        y++;
        err += 1 + 2*y;
        if (2*(err - x) + 1 > 0) {
            x--;
            err += 1 - 2*x;
        }
    }
}

void gfx_fill_circle(layer_id_t layer,  uint32_t cx, uint32_t cy, uint32_t radius, argb_t color) {
    const int r = radius;
    const int r2 = r * r;
    const int edge = 2;

    for (int y = -r - 1; y <= r + 1; y++) {
        for (int x = -r - 1; x <= r + 1; x++) {
            int dx = x;
            int dy = y;
            int dist2 = dx * dx + dy * dy;

            if (dist2 <= r2 - edge) {
                gfx_draw_pixel(layer, cx + dx, cy + dy, color);
            } else if (dist2 <= r2 + edge) {
                int diff = r2 + edge - dist2;
                uint8_t alpha = (uint8_t)((diff * 255) / (2 * edge));
                blend_pixel_layer(layer, cx + dx, cy + dy, color, alpha);
            }
        }
    }
}

void gfx_draw_polygon(layer_id_t layer, const uint32_t* xs, const uint32_t* ys, size_t count, argb_t color) {
    uint32_t lx = xs[0];
    uint32_t ly = ys[0];

    for (size_t i = 1; i < count; i++) {
        gfx_draw_line(layer, lx, ly, xs[i], ys[i], color);

        lx = xs[i];
        ly = ys[i];
    }

    gfx_draw_line(layer, lx, ly, xs[0], ys[0], color);
}

void gfx_clear(layer_id_t layer, argb_t color) {
    uint32_t total = fb_info.pixels_per_row * fb_info.height;
    for (uint32_t i = 0; i < total; i++) {
        layers[layer][i] = color;
    }

    dirty_rects[layer][0] = (gfx_dirty_rect_t){
        .x_min = 0,
        .y_min = 0,
        .x_max = fb_info.width - 1,
        .y_max = fb_info.height - 1,
        .dirty = true,
    };
}

void gfx_flush_dirty(void) {
    gfx_dirty_rect_t final_rects[MAX_DIRTY_RECTS];
    int final_count = 0;

    for (layer_id_t l = 0; l < LAYER_COUNT; l++) {
        for (int i = 0; i < MAX_DIRTY_RECTS; i++) {
            if (!dirty_rects[l][i].dirty) {
                continue;
            }
            gfx_dirty_rect_t r = dirty_rects[l][i];

            // Merge with existing final_rects
            bool merged = false;
            for (int j = 0; j < final_count; j++) {
                gfx_dirty_rect_t* fr = &final_rects[j];
                if (!(r.x_max < fr->x_min || r.x_min > fr->x_max || r.y_max < fr->y_min || r.y_min > fr->y_max)) {
                    fr->x_min = fr->x_min < r.x_min ? fr->x_min : r.x_min;
                    fr->y_min = fr->y_min < r.y_min ? fr->y_min : r.y_min;
                    fr->x_max = fr->x_max > r.x_max ? fr->x_max : r.x_max;
                    fr->y_max = fr->y_max > r.y_max ? fr->y_max : r.y_max;
                    merged = true;
                    break;
                }
            }

            if (!merged && final_count < MAX_DIRTY_RECTS) {
                final_rects[final_count++] = r;
            }

            dirty_rects[l][i].dirty = false;
        }
    }

    for (int i = 0; i < final_count; i++) {
        gfx_dirty_rect_t* r = &final_rects[i];
        int w = r->x_max - r->x_min + 1;
        int h = r->y_max - r->y_min + 1;

        for (int row = 0; row < h; row++) {
            for (int col = 0; col < w; col++) {
                int x = r->x_min + col;
                int y = r->y_min + row;
                int offset = y * fb_info.pixels_per_row + x;

                argb_t color = layers[LAYER_BACKGROUND][offset];
                color = blend_argb(color, layers[LAYER_UI][offset]);
                color = blend_argb(color, layers[LAYER_OVERLAY][offset]);
                color = blend_argb(color, layers[LAYER_CURSOR][offset]);

                double_buffer[offset] = color;
            }
        }

        for (int row = 0; row < h; row++) {
            uint32_t* dst = &((uint32_t*)fb_info.addr)[(r->y_min + row) * fb_info.pixels_per_row + r->x_min];
            uint32_t* src = &double_buffer[(r->y_min + row) * fb_info.pixels_per_row + r->x_min];
            kmemcpy(dst, src, w * sizeof(argb_t));
        }
    }
}

void gfx_flush(void) {
    fb_flush(double_buffer);
}

void gfx_mark_dirty(layer_id_t layer, int x, int y) {
    if (x >= (int)fb_info.width || y >= (int)fb_info.height) { 
        return;
    }

    const int padding = 1;

    int x0 = x - padding;
    int y0 = y - padding;
    int x1 = x + padding;
    int y1 = y + padding;

    int w = (int) fb_info.width;
    int h = (int) fb_info.height;

    x0 = x0 < 0 ? 0 : x0;
    y0 = y0 < 0 ? 0 : y0;
    x1 = x1 >= w ? w - 1 : x1;
    y1 = y1 >= h ? h - 1 : y1;

    // Try to merge with existing rect
    for (int i = 0; i < MAX_DIRTY_RECTS; i++) {
        if (!dirty_rects[layer][i].dirty) {
            continue;
        }

        gfx_dirty_rect_t* r = &dirty_rects[layer][i];
        if (!(x1 < r->x_min || x0 > r->x_max || y1 < r->y_min || y0 > r->y_max)) {
            r->x_min = r->x_min < x0 ? r->x_min : x0;
            r->y_min = r->y_min < y0 ? r->y_min : y0;
            r->x_max = r->x_max > x1 ? r->x_max : x1;
            r->y_max = r->y_max > y1 ? r->y_max : y1;
            
            return;
        }
    }

    // Add new rect
    for (int i = 0; i < MAX_DIRTY_RECTS; i++) {
        if (!dirty_rects[layer][i].dirty) {
            dirty_rects[layer][i] = (gfx_dirty_rect_t){ x0, y0, x1, y1, true };
            return;
        }
    }
}

static inline argb_t blend_argb(argb_t bottom, argb_t top) {
    if ((top >> 24) == 0) {
        return bottom;
    }

    uint8_t alpha = top >> 24;
    uint8_t sr = (top >> 16) & 0xFF;
    uint8_t sg = (top >> 8) & 0xFF;
    uint8_t sb = top & 0xFF;

    uint8_t br = (bottom >> 16) & 0xFF;
    uint8_t bg = (bottom >> 8) & 0xFF;
    uint8_t bb = bottom & 0xFF;

    uint8_t r = (sr * alpha + br * (255 - alpha)) / 255;
    uint8_t g = (sg * alpha + bg * (255 - alpha)) / 255;
    uint8_t b = (sb * alpha + bb * (255 - alpha)) / 255;

    return (alpha << 24) | (r << 16) | (g << 8) | b;
}

static void blend_pixel_layer(layer_id_t layer, uint32_t x, uint32_t y, argb_t color, uint8_t alpha) {
    if (x >= fb_info.width || y >= fb_info.height) { 
        return;
    }

    uint32_t offset = y * fb_info.pixels_per_row + x;
    argb_t dst = layers[layer][offset];

    uint8_t src_r = (color >> 16) & 0xFF;
    uint8_t src_g = (color >> 8) & 0xFF;
    uint8_t src_b = color & 0xFF;

    uint8_t dst_r = (dst >> 16) & 0xFF;
    uint8_t dst_g = (dst >> 8) & 0xFF;
    uint8_t dst_b = dst & 0xFF;

    uint8_t out_r = (src_r * alpha + dst_r * (255 - alpha)) / 255;
    uint8_t out_g = (src_g * alpha + dst_g * (255 - alpha)) / 255;
    uint8_t out_b = (src_b * alpha + dst_b * (255 - alpha)) / 255;

    layers[layer][offset] = (alpha << 24) | (out_r << 16) | (out_g << 8) | out_b;
    gfx_mark_dirty(layer, x, y);
}
