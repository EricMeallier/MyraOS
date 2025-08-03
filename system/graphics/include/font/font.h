#ifndef FONT_H
#define FONT_H

#include <stdint.h>
#include <stdarg.h>

#include "fb/fb.h"

typedef struct font_t {
    uint8_t width;
    uint8_t height;
    const uint8_t* data;
} font_t;

typedef struct cursor_t {
    uint32_t x;
    uint32_t y;
} cursor_t;

typedef struct box_limit {
    uint32_t x, y;
    uint32_t width, height;
} box_limit_t;

typedef struct font_state_t {
    cursor_t cursor;
    font_t* font;
    argb_t color;
    box_limit_t box_limit;
} font_state_t;

void font_init_default(font_t* default_font);

font_state_t font_save_state(void);
void font_restore_state(font_state_t state);

void font_set_font(font_t* new_font);
font_t* font_get_font(void);

void font_set_box_limit(box_limit_t* box_limit);
box_limit_t font_get_box_limit(void);

void font_set_cursor(cursor_t c);
cursor_t font_get_cursor(void);

void font_set_color(argb_t color);
argb_t font_get_color(void);

void font_set_background_color(argb_t color) ;
argb_t font_get_background_color(void);

void font_scroll(uint32_t lines);

void font_write_char(char c);
void font_write_char_at(char c, uint32_t x, uint32_t y);
void font_write(const char* str);
void font_writef(const char* fmt, ...);
void font_write_format(const char* fmt, va_list ap); 
void font_clear(argb_t color);

#endif // FONT_H