#include "vga/vga.h"

#include "vga/vga_colors.h"
#include "io/port_io.h"

#include <stddef.h>
#include <stdbool.h>

volatile uint16_t *video_memory = VGA_VIDEO_MEMORY;
volatile uint8_t color = VGA_COLOR_WHITE;

bool check_for_escape_chars(const uint16_t c, uint16_t cursor_pos);

void vga_clear(void) {
    const uint16_t blank = ' ' | color << 8;

    for (size_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        video_memory[i] = blank;
    }

    vga_set_cursor(0);
}

void vga_put_char(const char c) {
    uint16_t cursor_pos = vga_get_cursor();
    const uint16_t char_with_color = c | color << 8;

    if (cursor_pos == VGA_WIDTH * VGA_HEIGHT) {
        vga_scroll(1);
        cursor_pos -= VGA_WIDTH;
    }

    if (check_for_escape_chars(c, cursor_pos)) {
        return;
    }
        
    video_memory[cursor_pos] = char_with_color;
    vga_set_cursor(cursor_pos + 1);
}

void vga_write(const char *str) {
    for (size_t i = 0; str[i] != 0; i++) {
        vga_put_char(str[i]);
    }
}

void vga_write_format(const char *fmt, const va_list argp) {
    while (*fmt) {
        if (*fmt != '%') {
            vga_put_char(*fmt);
        } else {
            fmt++;

            switch (*fmt) {
                case 'd': {
                    int val = va_arg(argp, int);
                    vga_write_int(val);

                    break;
                }
                case 's': {
                    const char *str = va_arg(argp, const char *);
                    vga_write(str);

                    break;
                }
                case 'c': {
                    char c = (char)va_arg(argp, int);
                    vga_put_char(c);

                    break;
                }
                case 'x': {
                    uint32_t val = va_arg(argp, uint32_t);
                    vga_write_hex(val);

                    break;
                }
                case '%': {
                    vga_put_char('%');

                    break;
                }
                default: {
                    vga_put_char('%');
                    vga_put_char(*fmt);

                    break;
                }
            }
        }

        fmt++;
    }
}

void vga_write_int(int num) {
    char buf[12];
    int i = 0;
    bool is_negative = false;

    if (num == 0) {
        vga_put_char('0');
        return;
    }

    if (num < 0) {
        is_negative = true;
        num = -num;
    }

    while (num > 0) {
        buf[i++] = '0' + (num % 10);
        num /= 10;
    }

    if (is_negative) {
        buf[i++] = '-';
    }

    // reverse the buffer
    while (--i >= 0) {
        vga_put_char(buf[i]);
    }
}

void vga_write_hex(const uint32_t num) {
    vga_write("0x");

    bool started = false;
    for (int i = 28; i >= 0; i -= 4) {
        uint8_t nibble = (num >> i) & 0xF;

        if (nibble == 0 && !started && i != 0) {
            // skip leading zeroes
            continue; 
        }

        started = true;
        if (nibble < 10) {
            vga_put_char('0' + nibble);
        } else {
            vga_put_char('A' + (nibble - 10));
        }
    }

    if (!started) {
        vga_put_char('0');
    }
}

bool check_for_escape_chars(const uint16_t c, uint16_t cursor_pos) {
    switch (c) {
        case '\n':
            cursor_pos = cursor_pos + (VGA_WIDTH - cursor_pos % VGA_WIDTH);
            break;
        case '\t':
            cursor_pos = cursor_pos + 4;
            break;
        default:
            return false;
    }

    vga_set_cursor(cursor_pos);
    return true;
}

void vga_set_color(const uint8_t new_color) {
    color = new_color;
}

void vga_set_cursor(const uint16_t cursor_pos) {
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(cursor_pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)(cursor_pos >> 8 & 0xFF));
}

uint16_t vga_get_cursor(void) {
    uint16_t cursor_pos = 0;

    outb(0x3D4, 0x0F);
    cursor_pos |= inb(0x3D5);
    outb(0x3D4, 0x0E);
    cursor_pos |= (uint16_t) inb(0x3D5) << 8;

    return cursor_pos;
}

void vga_scroll(const uint8_t lines) {
    // clear first n lines
    const uint16_t blank = ' ' | color << 8;
    for (size_t i = 0; i < lines * VGA_WIDTH; i++) {
        video_memory[i] = blank;
    }

    // transfer each line to the top free place
    for (size_t i = lines * VGA_WIDTH; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        video_memory[i - lines * VGA_WIDTH] = video_memory[i];
        video_memory[i] = blank;
    }

    vga_set_cursor(vga_get_cursor() - VGA_WIDTH * lines);
}

