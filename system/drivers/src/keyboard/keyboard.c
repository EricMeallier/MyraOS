#include "keyboard/keyboard.h"

#include "interrupt/irq/irq.h"
#include "io/port_io.h"
#include "keyboard/scan_codes.h"
#include "vga/vga.h"

#include <stdbool.h>

void keyboard_set_scan_code_set(uint8_t set) {
    // Wait until input buffer is clear
    while (inb(0x64) & 0x02);
    outb(0x60, 0xF0); // Set Scan Code Set

    while (inb(0x64) & 0x02);
    outb(0x60, set);

    // Read ACKs (0xFA expected)
    inb(0x60); // ack for 0xF0
    inb(0x60); // ack for 0x02
}

void keyboard_driver_install(void) {
    keyboard_set_scan_code_set(KEYBOARD_SET);

    irq_install_handler(
        KEYBOARD_IRQ,
        keyboard_handler
    );
}

static bool extended = false;

void keyboard_handler(registers_t *regs) {
    // regs are pushed to the stack from the isr, but uneeded here
    (void)regs;

    const uint8_t scan_code = inb(0x60);

    if (scan_code == 0xE0) {
        extended = true;
        return;
    }

    // get the make code, scan code - without the msb
    uint8_t code = scan_code & 0x7F;
    bool released = scan_code & 0x80;

    if (released) {
        return;
    }

    char ch = 0;

    if (extended) {
        extended = false;
        return;
    }

    // change to the cb in the future
    ch = keyboard_scan_codes[code];
    if (ch) {
        vga_put_char(ch);
    }
}