#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include <stdbool.h>
#include <stddef.h>


#define BUFFER_SIZE 256

typedef struct circular_buffer_t {
    char *data;
    size_t capacity;
    size_t count;
    size_t head;
    size_t tail;
} circular_buffer_t;

// init
void cb_init(circular_buffer_t *cb, size_t size);

// write
bool cb_write(circular_buffer_t *cb, char data);

// read
bool cb_read(circular_buffer_t *cb, char *out);

#endif  // CIRCULAR_BUFFER_H