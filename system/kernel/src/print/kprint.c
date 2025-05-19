//
// Created by dvir on 5/19/25.
//

#include "print/kprint.h"

#include "vga/vga.h"

void kprint(const char *str) {
    vga_write(str);
}

void kprintln(const char *str) {
    vga_write(str);
    vga_write("\n");
}

void kclear_screen(void) {
    vga_clear();
}
