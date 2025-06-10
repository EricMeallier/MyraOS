#include <stdbool.h>

#include "heap/heap.h"
#include "interrupt/idt/idt.h"
#include "interrupt/irq/irq.h"
#include "interrupt/isr/isr.h"
#include "keyboard/keyboard.h"
#include "pmm/pmm.h"
#include "print/kprint.h"
#include "shell.h"
#include "vmm/vmm.h"

void kernel_main() {
    kclear_screen();

    pmm_init();
    vmm_init();

    heap_init(HEAP_START_ADDR, HEAP_SIZE);

    idt_init();
    isr_install();
    irq_install();

    keyboard_driver_install();

    kprint("Welcome to ");
    kset_color(COLOR_GREEN);
    kprint("MyraOS");
    kset_color(COLOR_WHITE);
    kprintln("!");

    shell_run();

    while (true) {
        __asm__ volatile("hlt");
    }
}

__attribute__((section(".text._start"))) void _start() { kernel_main(); }