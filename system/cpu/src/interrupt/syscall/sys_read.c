#include "keyboard/keyboard.h"

#define STDIN 0

uint32_t sys_read(uint32_t fd, uint32_t buf_ptr, uint32_t len, uint32_t _) {
    (void)_; // unused

    if (fd != STDIN) {
        return -1; // Only support stdin
    }

    char* buf = (char*) buf_ptr;
    uint32_t read = 0;

    while (read < len) {
        key_event_t event;
        if (keyboard_read_event(&event) && event.ascii_value != 0 && event.pressed) {
            buf[read++] = event.ascii_value;

            if (event.ascii_value == '\n') {
                break;
            }
        }
    }

    return read;
}
