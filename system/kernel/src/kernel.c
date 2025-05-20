#include <stdbool.h>

#include "print/kprint.h"
#include "interrupt/idt/idt.h"
#include <interrupt/isr/isr.h>

void kernel_main() {
    idt_init();

    kclear_screen();

    kprint("Welcome to ");
    kset_color(COLOR_GREEN);
    kprint("MyraOS");
    kset_color(COLOR_WHITE);
    kprintln("!");

    __asm__ volatile("int $0");

    while (true) {
        __asm__ volatile("hlt");
    }
}

__attribute__((section(".text._start")))
void _start() {
    kernel_main();
}