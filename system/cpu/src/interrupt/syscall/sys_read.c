#include "keyboard/keyboard.h"
#include "tty/tty.h"
#include "libc_kernel/string.h"

#define STDIN 0

uint32_t sys_read(uint32_t fd, uint32_t buf_ptr, uint32_t len, uint32_t _) {
    (void)_; // unused

    if (fd != STDIN) {
        return -1;
    }

    while (!tty_read_line((char*) buf_ptr, len)) {
        __asm__ volatile ("hlt");
    }

    return kstrlen((char*) buf_ptr);
}
