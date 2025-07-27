#ifndef TTY_H
#define TTY_H

#include <stdint.h>
#include <stdbool.h>

#define TTY_LINE_CAPACITY 256
#define TTY_LINE_COUNT 16

typedef struct tty_line_t {
    char line[TTY_LINE_CAPACITY];
} tty_line_t;

void tty_init(void);
void tty_handle_char(char c);
bool tty_read_line(char* out, uint32_t max_len);

#endif // TTY_H