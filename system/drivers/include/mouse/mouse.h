#ifndef MOUSE_H
#define MOUSE_H

#include <stdint.h>
#include <stdbool.h>

#define MOUSE_IRQ 12

#define CURSOR_SIZE 5
#define CURSOR_SAFE_MARGIN 1
#define CURSOR_AREA_SIZE (CURSOR_SIZE + CURSOR_SAFE_MARGIN)
#define CURSOR_COLOR 0xFFFFFFFF

void mouse_init(void);

void mouse_write(uint8_t val);
uint8_t mouse_read(void);

void mouse_set(bool is_enabled);

#endif // MOUSE_H