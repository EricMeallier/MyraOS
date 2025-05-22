#include <stdbool.h>

#include "print/kprint.h"
#include "vga/vga.h"
#include "io/port_io.h"
#include "interrupt/idt/idt.h"
#include "interrupt/isr/isr.h"
#include "interrupt/irq/irq.h"

void kernel_main() {
    idt_init();
    isr_install();
    irq_install();

    kclear_screen();

    kprint("Welcome to ");
    kset_color(COLOR_GREEN);
    kprint("MyraOS");
    kset_color(COLOR_WHITE);
    kprintln("!");

    __asm__ volatile("sti");

    while (true) {
        __asm__ volatile("hlt");
    }
}

__attribute__((section(".text._start")))
void _start() {
    kernel_main();
}