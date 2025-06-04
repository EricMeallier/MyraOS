//
// Created by dvir on 5/22/2025.
//

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "interrupt/idt/idt.h"

#define KEYBOARD_SCAN_CODES_SIZE 128
#define KEYBOARD_IRQ 1
#define KEYBOARD_SET 2

extern const char keyboard_scan_codes[KEYBOARD_SCAN_CODES_SIZE];

// install
extern void keyboard_driver_install(void);

// handler
extern void keyboard_handler(registers_t* regs);

#endif // KEYBOARD_H
