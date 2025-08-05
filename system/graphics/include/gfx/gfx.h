#ifndef GFX_H
#define GFX_H

#include "fb/fb.h"

#include <stddef.h>

typedef enum {
    LAYER_BACKGROUND,
    LAYER_UI,
    LAYER_OVERLAY,
    LAYER_CURSOR,
    LAYER_COUNT
} layer_id_t;

void gfx_init(void);

void gfx_draw_pixel(layer_id_t layer, uint32_t x, uint32_t y, argb_t color);
argb_t gfx_get_pixel(uint32_t x, uint32_t y);
void gfx_draw_line(layer_id_t layer, uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, argb_t color);
void gfx_draw_rect(layer_id_t layer, uint32_t x, uint32_t y, uint32_t width, uint32_t height, argb_t color);
void gfx_fill_rect(layer_id_t layer, uint32_t x, uint32_t y, uint32_t width, uint32_t height, argb_t color);
void gfx_draw_circle(layer_id_t layer, uint32_t cx, uint32_t cy, uint32_t radius, argb_t color);
void gfx_fill_circle(layer_id_t layer, uint32_t cx, uint32_t cy, uint32_t radius, argb_t color);
void gfx_draw_polygon(layer_id_t layer, const uint32_t* xs, const uint32_t* ys, size_t count, argb_t color);

void gfx_clear(layer_id_t layer, argb_t color);

void gfx_flush_dirty(void);
void gfx_flush(void);

void gfx_mark_dirty(layer_id_t layer, int x, int y);

#endif // GFX_H