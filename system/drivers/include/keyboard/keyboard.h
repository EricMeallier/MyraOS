//
// Created by dvir on 5/22/2025.
//

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdbool.h>

#include "interrupt/idt/idt.h"
#include "keyboard/make_codes.h"

#define KEYBOARD_SCAN_CODES_SIZE 128
#define KEYBOARD_IRQ 1

extern const char keyboard_scan_codes[KEYBOARD_SCAN_CODES_SIZE];
extern const char keyboard_shift_scan_codes[KEYBOARD_SCAN_CODES_SIZE];

typedef struct key_event_t {
    make_code_t make_code;      
    char ascii_value;
    bool pressed;
    bool extended;
    bool ctrl, shift, alt;
} key_event_t;

// install
extern void keyboard_driver_install(void);

// handler
extern void keyboard_handler(registers_t* regs);

#endif  // KEYBOARD_H
