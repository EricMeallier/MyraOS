#include "interrupt/isr/isr.h"

#include "print/kprint.h"

#include <stdbool.h>

void isr_handler(const registers_t* regs) {
    kclear_screen();
    kset_color(COLOR_RED);

    kprintln("Exception!");

    if (regs->interrupt < ISR_EXCEPTION_AMOUNT) {
        kprintln(exception_messages[regs->interrupt]);
    } else {
        kprintln("Unknown Exception");
    }

    kset_color(COLOR_WHITE);

    while (true) {
        __asm__ volatile("hlt");
    }
}
