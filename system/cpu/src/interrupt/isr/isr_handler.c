#include "interrupt/isr/isr.h"

#include "print/kprint.h"

#include <stdbool.h>

void isr_handler(uint32_t int_num, uint32_t error_code) {
    kclear_screen();
    kset_color(COLOR_RED);

    kprintln("Exception!");

    vga_write_int(int_num);
    vga_write_int(error_code);

    if (int_num < ISR_EXCEPTION_AMOUNT) {
        kprintln(exception_messages[int_num]);
    } else {
        kprintln("Unknown Exception");
    }

    kset_color(COLOR_WHITE);

    while (true) {
        __asm__ volatile("hlt");
    }
}
