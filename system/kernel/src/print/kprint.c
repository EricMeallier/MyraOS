//
// Created by dvir on 5/19/25.
//

#include "print/kprint.h"

#include "vga/vga.h"
#include "vga/vga_colors.h"

#include <stdint.h>

void kprint(const char *str) {
    vga_write(str);
}

void kprintln(const char *str) {
    vga_write(str);
    vga_write("\n");
}

void kprintf(const char *fmt, ...) {
    va_list argp;
    va_start(argp, fmt);

    vga_write_format(fmt, argp);

    va_end(argp);
}

void kclear_screen(void) {
    vga_clear();
}

void kset_color(const color_t color) {
    uint8_t vga_color;

    if (color == COLOR_WHITE) {
        vga_color = VGA_COLOR_WHITE;
    }

    if (color == COLOR_GREEN) {
        vga_color = VGA_COLOR_GREEN;
    }

    if (color == COLOR_RED) {
        vga_color = VGA_COLOR_RED;
    }

    vga_set_color(vga_color);
}
