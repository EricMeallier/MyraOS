#ifndef GFX_H
#define GFX_H

#include "fb/fb.h"

#include <stddef.h>

void gfx_init(void);

void gfx_draw_pixel(uint32_t x, uint32_t y, argb_t color);
argb_t gfx_get_pixel(uint32_t x, uint32_t y);
void gfx_blend_pixel(int x, int y, argb_t color, uint8_t alpha);
void gfx_draw_line(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, argb_t color);
void gfx_draw_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, argb_t color);
void gfx_fill_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, argb_t color);
void gfx_draw_circle(uint32_t cx, uint32_t cy, uint32_t radius, argb_t color);
void gfx_fill_circle(uint32_t cx, uint32_t cy, uint32_t radius, argb_t color);
void gfx_draw_polygon(const uint32_t* xs, const uint32_t* ys, size_t count, argb_t color);

void gfx_clear(argb_t color);

void gfx_flush(void);

#endif // GFX_H