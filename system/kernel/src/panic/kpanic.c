#include "panic/kpanic.h"

#include <stdbool.h>

#include "interrupt/isr/isr.h"
#include "print/kprint.h"

void kpanic(const char* message) {
    kclear_screen();
    kset_color(COLOR_RED);

    kprintln("KERNEL PANIC!");
    kprintln(message);

    kset_color(COLOR_WHITE);

    while (true) {
        __asm__ volatile("hlt");
    }
}