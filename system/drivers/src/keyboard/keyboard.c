#include "keyboard/keyboard.h"

#include "interrupt/irq/irq.h"
#include "io/port_io.h"
#include "vga/vga.h"

static bool extended = false;

static uint8_t is_shift_pressed = 0;
static uint8_t is_ctrl_pressed = 0;
static uint8_t is_alt_pressed = 0;

static bool is_caps_lock_on = false;

static key_event_t create_key_event(uint8_t scan_code, bool released);
static void update_modifier_state(uint8_t make_code, bool released);

void keyboard_driver_install(void) {
    irq_install_handler(KEYBOARD_IRQ, keyboard_handler);
}

void keyboard_handler(registers_t *regs) {
    // regs are pushed to the stack from the isr, but unneeded here
    (void)regs;

    const uint8_t scan_code = inb(0x60);

    if (scan_code == 0xE0) {
        extended = true;
        return;
    }

    // get the make code, scan code - without the msb
    uint8_t make_code = scan_code & 0x7F;
    bool released = scan_code & 0x80;

    if (extended) {
        extended = false;
    }

    if (make_code == KEY_CAPS_LOCK && !released) {
        is_caps_lock_on = !is_caps_lock_on;
        return;
    }

    update_modifier_state(make_code, released);

    if (released) {
        return;
    }

    key_event_t event = create_key_event(scan_code, released);

    // TODO: change to cb
    if (event.ascii_value >= 32 || event.ascii_value == 10) {
        vga_put_char(event.ascii_value);
    } else if (event.ascii_value) {
        vga_put_char('^');
        vga_put_char(event.ascii_value + 64);
    }
}

static void update_modifier_state(uint8_t make_code, bool released) {
    if (make_code == KEY_LEFT_SHIFT || make_code == KEY_RIGHT_SHIFT) {
        released ? is_shift_pressed-- : is_shift_pressed++;
    } else if (make_code == KEY_LEFT_CTRL || (KEY_RIGHT_CTRL && extended)) {
        released ? is_ctrl_pressed-- : is_ctrl_pressed++;
    } else if (make_code == KEY_LEFT_ALT) {
        released ? is_alt_pressed-- : is_alt_pressed++; 
    }
}

static key_event_t create_key_event(uint8_t scan_code, bool released) {
    key_event_t e;

    e.make_code = scan_code & 0x7F;
    e.pressed = !released;
    e.extended = extended;
    e.ctrl = is_ctrl_pressed;
    e.shift = is_shift_pressed;
    e.alt = is_alt_pressed;

    char ch = 0;
    if (!is_shift_pressed) {
        ch = keyboard_scan_codes[e.make_code];

        if (is_caps_lock_on && ch >= 'a' && ch <= 'z') {
            ch -= 32; // convert to uppercase
        } 
    } else {
        ch = keyboard_shift_scan_codes[e.make_code];

        if (is_caps_lock_on && ch >= 'A' && ch <= 'Z') {
            ch += 32; // convert to lowercase
        } 
    }

    if (is_ctrl_pressed) {
        // convert to control char form
        if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')) {
            ch = (ch & 0x1F);
        } else {
            ch = 0;
        }
    }

    e.ascii_value = ch;

    return e;
}
