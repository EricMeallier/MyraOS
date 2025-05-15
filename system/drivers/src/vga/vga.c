#include "vga/vga.h"

#include "vga/vga_colors.h"
#include "io/port_io.h"

volatile uint16_t *video_memory = VGA_VIDEO_MEMORY;
volatile uint8_t color = VGA_COLOR_WHITE_ON_BLACK;

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

    if (c == '\n') {
        const uint16_t new_cursor_pos = cursor_pos + (VGA_WIDTH - cursor_pos % VGA_WIDTH);
        vga_set_cursor(new_cursor_pos);

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