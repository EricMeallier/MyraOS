#include "input/input.h"

#include "keyboard/keyboard.h"
#include "tty/tty.h"

#include "print/print.h"

void input_process(void) {
    key_event_t e;
    if (keyboard_read_event(&e) && e.pressed) {
        tty_handle_char(e.ascii_value);
    }
}
