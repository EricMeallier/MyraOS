#include "circular_buffer/circular_buffer.h"

#include "heap/heap.h"

void cb_init(circular_buffer_t *cb, size_t size) {
    cb->capacity = size;
    cb->data = (char*) kmalloc(size * sizeof(char));

    cb->head = 0;
    cb->tail = 0;
    cb->count = 0;
}

bool cb_write(circular_buffer_t *cb, char data) {
    if (cb->count == cb->capacity) {
        return false;
    }

    cb->data[cb->head] = data;

    cb->head = (cb->head + 1) % cb->capacity;
    cb->count++;
    
    return true;
}

bool cb_read(circular_buffer_t *cb, char *out) {
    if (cb->capacity == 0 || cb->count == 0) {
        return false;
    }

    *out = cb->data[cb->tail];

    cb->tail = (cb->tail + 1) % cb->capacity;
    cb->count--;

    return true;
}
