#include <stdbool.h>

#include "print/kprint.h"
#include "interrupt/idt/idt.h"
#include "interrupt/isr/isr.h"
#include "interrupt/irq/irq.h"
#include "keyboard/keyboard.h"
#include "pmm/pmm.h"

void kernel_main() {
    kclear_screen();

    pmm_init();

    idt_init();
    isr_install();
    irq_install();
    
    keyboard_driver_install();

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