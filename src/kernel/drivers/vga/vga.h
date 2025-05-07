#pragma once

#include <stdint.h>
#include <stddef.h>

#define VGA_VIDEO_MEMORY (uint16_t*) 0xB8000

#define VGA_WIDTH 80
#define VGA_HEIGHT 25

extern volatile uint16_t *video_memory;
extern volatile uint8_t color;

// clear
void vga_clear(void);

// write
void vga_put_char(char c);
void vga_write(const char *str);

// colors
void vga_set_color(uint8_t new_color);

// cursor
void vga_set_cursor(uint16_t pos);
uint16_t vga_get_cursor(void);
