#include <stdbool.h>

#include "vga/vga.h"
#include "vga/vga_colors.h"
#include "print/kprint.h"

void kernel_main() {
    kclear_screen();

    kprint("Welcome to ");
    kset_color(COLOR_GREEN);
    kprint("MyraOS");
    kset_color(COLOR_WHITE);
    kprintln("!");

    while (true) {
        __asm__ volatile("hlt");
    }
}

__attribute__((section(".text._start")))
void _start() {
    kernel_main();
}