#include <stdbool.h>

#include "drivers/vga/vga.h"
#include "drivers/vga/vga_colors.h"

void kernel_main() {
    vga_clear();

    vga_write("Welcome to ");
    vga_set_color(VGA_COLOR_GREEN_ON_BLACK);
    vga_write("MyraOS");
    vga_set_color(VGA_COLOR_WHITE_ON_BLACK);
    vga_write("!");

    while (true) {
        __asm__ volatile("hlt");
    }
}

void _start() {
    kernel_main();
}