#include "font/font.h"

#include <stdbool.h>

#include "gfx/gfx.h"
#include "libc_kernel/stdlib.h"
#include "libc_kernel/string.h"

#define FONT_FIRST_READABLE_ASCII 32
#define FONT_ASCII_COUNT 95

static bool font_handle_escape_char(char c);

static cursor_t current_cursor;
static box_limit_t current_box_limit;
static argb_t font_color;
static argb_t background_color;
static font_t* current_font;

void font_set_font(font_t* new_font) {
    current_font = new_font;
}

void font_set_box_limit(box_limit_t* box_limit) {
    current_box_limit = (box_limit_t) {
        .height = box_limit->height,
        .width = box_limit->width,
        .x = box_limit->x,
        .y = box_limit->y,
    };
}

void font_set_cursor(cursor_t c) {
    current_cursor = c;
}

cursor_t font_get_cursor(void) {
    return current_cursor;
}

void font_set_color(argb_t color) {
    font_color = color;
}

void font_set_background_color(argb_t color) {
    background_color = color;
}

void font_scroll(uint32_t lines) {
    uint32_t dy = lines * current_font->height;
    for (uint32_t y = current_box_limit.y; y < current_box_limit.y + current_box_limit.height - dy; y++) {
        for (uint32_t x = current_box_limit.x; x < current_box_limit.x + current_box_limit.width; x++) {
            gfx_draw_pixel(x, y, gfx_get_pixel(x, y + dy));
        }
    }

    // Clear the new space at the bottom
    for (uint32_t y = current_box_limit.y + current_box_limit.height - dy;
         y < current_box_limit.y + current_box_limit.height; y++) {
        for (uint32_t x = current_box_limit.x; x < current_box_limit.x + current_box_limit.width; x++) {
            gfx_draw_pixel(x, y, background_color);
        }
    }
}

void font_write_char(char c) {
    uint32_t char_width = current_font->width;
    uint32_t char_height = current_font->height;
    
    if (current_cursor.x + char_width > current_box_limit.x + current_box_limit.width) {
        current_cursor.x = current_box_limit.x;
        current_cursor.y += char_height;
        
        if (current_cursor.y + char_height > current_box_limit.y + current_box_limit.height) {
            font_scroll(1);
            current_cursor.y -= char_height;
        }
    }

    uint32_t bytes_per_row = (char_width + 7) / 8;
    if ((uint8_t)c < FONT_FIRST_READABLE_ASCII || (uint8_t)c >= FONT_FIRST_READABLE_ASCII + FONT_ASCII_COUNT) {
        return;
    }

    uint8_t char_index = (uint8_t)c - FONT_FIRST_READABLE_ASCII;
    const uint8_t* char_bitmap = &current_font->data[char_index * char_height * bytes_per_row];

    for (uint32_t y = 0; y < char_height; y++) {
        for (uint32_t x = 0; x < char_width; x++) {
            uint32_t byte_index = y * bytes_per_row + (x / 8);
            uint8_t bit_mask = 0x80 >> (x % 8);
            if (char_bitmap[byte_index] & bit_mask) {
                gfx_draw_pixel(current_cursor.x + x, current_cursor.y + y, font_color);
            }
        }
    }

    current_cursor.x += char_width;
}

void font_write_char_at(char c, uint32_t x, uint32_t y) {
    cursor_t prev = current_cursor;

    font_set_cursor((cursor_t){x, y});
    font_write_char(c);

    font_set_cursor(prev);
}

void font_write(const char* str) {
    while (*str) {
        char c = *str++;
        if (font_handle_escape_char(c)) {
            continue;
        }

        font_write_char(c);
    }
}

void font_write_format(const char* fmt, va_list ap) {
    while (*fmt) {
        if (*fmt != '%') {
            font_write_char(*fmt++);
            continue;
        }

        fmt++; // Skip '%'

        bool zero_pad = false;
        int pad_width = 0;

        if (*fmt == '0') {
            zero_pad = true;
            fmt++;
        }

        while (*fmt >= '0' && *fmt <= '9') {
            pad_width = pad_width * 10 + (*fmt - '0');
            fmt++;
        }

        char buf[32];

        switch (*fmt) {
            case 'd': {
                int val = va_arg(ap, int);
                kitoa(val, buf, 10);

                int len = kstrlen(buf);
                for (int i = 0; i < pad_width - len; i++) {
                    font_write_char(zero_pad ? '0' : ' ');
                }

                font_write(buf);
                break;
            }
            case 'x': {
                uint32_t val = va_arg(ap, uint32_t);
                kitoa(val, buf, 16);

                int len = kstrlen(buf);
                for (int i = 0; i < pad_width - len; i++) {
                    font_write_char(zero_pad ? '0' : ' ');
                }

                font_write(buf);
                break;
            }
            case 'u': {
                uint32_t val = va_arg(ap, uint32_t);
                utoa(val, buf, 10);

                int len = kstrlen(buf);
                for (int i = 0; i < pad_width - len; i++) {
                    font_write_char(zero_pad ? '0' : ' ');
                }

                font_write(buf);
                break;

            }
            case 's': {
                const char* str = va_arg(ap, const char*);
                font_write(str);
                break;
            }
            case 'c': {
                char c = (char)va_arg(ap, int);
                font_write_char(c);
                break;
            }
            case '%': {
                font_write_char('%');
                break;
            }
            default: {
                font_write_char('%');
                font_write_char(*fmt);

                break;
            }
        }

        fmt++;
    }
}

void font_clear(argb_t color) {
    for (uint32_t y = current_box_limit.y; y < current_box_limit.y + current_box_limit.height; y++) {
        for (uint32_t x = current_box_limit.x; x < current_box_limit.x + current_box_limit.width; x++) {
            gfx_draw_pixel(x, y, color);
        }
    }
}

static bool font_handle_escape_char(char c) {
    switch (c) {
        case '\n': {
            current_cursor.x = current_box_limit.x;
            current_cursor.y += current_font->height;

            if (current_cursor.y + current_font->height > current_box_limit.y + current_box_limit.height) {
                font_scroll(1);
                current_cursor.y -= current_font->height;
            }

            return true;
        }
        case '\t': {
            uint32_t tab_size = 4;
            uint32_t next_tab = (current_cursor.x + tab_size) & ~(tab_size - 1);
            
            while (current_cursor.x < next_tab) {
                font_write_char(' ');
            }

            return true;
        }
        default:
            return false;
    }
}
