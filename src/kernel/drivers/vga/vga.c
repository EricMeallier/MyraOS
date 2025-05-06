#include "vga.h"

#include "vga_colors.h"
#include "kernel/io/port_io.h"

volatile uint16_t *video_memory = (uint16_t*) VGA_VIDEO_MEMORY;
volatile uint8_t color = VGA_COLOR_WHITE_ON_BLACK;

void vga_clear(void){
    const uint16_t blank = (' ' | (color << 8));

    for (size_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        video_memory[i] = blank;
    }

    vga_set_cursor(0);
}

void vga_put_char(char c) {
    const uint16_t cursor_pos = vga_get_cursor();
    const uint16_t char_with_color = (c | (color << 8));

    video_memory[cursor_pos] = char_with_color;
    vga_set_cursor(cursor_pos + 1);
}

void vga_write(const char *str) {
    size_t i = 0;
    for (; str[i] != 0; i++) {
        vga_put_char(str[i]);
    }
}

void vga_set_color(uint8_t new_color) {
    color = new_color;
}

void vga_set_cursor(uint16_t pos) {
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

uint16_t vga_get_cursor(void) {
    uint16_t pos = 0;

    outb(0x3D4, 0x0F);
    pos |= inb(0x3D5);
    outb(0x3D4, 0x0E);
    pos |= ((uint16_t)inb(0x3D5)) << 8;
    
    return pos;
}