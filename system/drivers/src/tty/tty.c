#include "tty/tty.h"

#include "circular_buffer/circular_buffer.h"
#include "print/print.h"
#include "libc_kernel/string.h"

#define IS_CHAR_BACKSPACE(c) ((c == 0x08 || c == 0x7F) && current_len > 0)
#define IS_CHAR_WRITABLE(c) (c >= 0x20)

static circular_buffer_t tty_line_buffer;
static tty_line_t current_line;
static uint32_t current_len = 0;

void tty_init(void) {
    cb_init(&tty_line_buffer, sizeof(tty_line_t), TTY_LINE_COUNT);
}

void tty_handle_char(char c) {
    if (c == '\r') {
        c = '\n';
    }

    if (c == '\n') {
        if (current_len < TTY_LINE_CAPACITY - 1) {
            current_line.line[current_len++] = '\n';
        }
        current_line.line[current_len] = '\0';

        if (cb_write(&tty_line_buffer, &current_line)) {
            kprint("\n");
        }
        current_len = 0;

        return;
    }

    if (IS_CHAR_BACKSPACE(c)) {
        current_len--;
        kclear(1);

        return;
    }

    if (current_len < TTY_LINE_CAPACITY && IS_CHAR_WRITABLE(c)) {
        current_line.line[current_len++] = c;
        kprintf("%c", c);
    }
}

bool tty_read_line(char* out, uint32_t max_len) {
    tty_line_t line;
    if (!cb_read(&tty_line_buffer, &line)) {
        return false;
    }

    kstrncpy(out, line.line, max_len);
    out[max_len - 1] = '\0';

    return true;
}
