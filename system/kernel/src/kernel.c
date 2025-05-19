#include <stdbool.h>

#include "vga/vga.h"
#include "vga/vga_colors.h"
#include "print/kprint.h"

void kernel_main() {
    kclear_screen();

    kprint("Welcome to ");
    vga_set_color(VGA_COLOR_GREEN_ON_BLACK);
    kprint("MyraOS");
    vga_set_color(VGA_COLOR_WHITE_ON_BLACK);
    kprintln("!");

    while (true) {
        __asm__ volatile("hlt");
    }
}

__attribute__((section(".text._start")))
void _start() {
    kernel_main();
}