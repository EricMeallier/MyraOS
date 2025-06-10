#include "circular_buffer/circular_buffer.h"

#include "heap/heap.h"

// TEMP! until we get libc
void *memcpy(void* dest, const void* src, size_t n) {
    unsigned char *d = dest;
    const unsigned char* s = src;
    while (n--) {
        *d++ = *s++;
    }
    return dest;
}

void cb_init(circular_buffer_t* cb, size_t element_size, size_t capacity) {
    cb->element_size = element_size;
    cb->capacity = capacity;
    cb->data = kmalloc(element_size * capacity);
    cb->head = cb->tail = cb->count = 0;
}

bool cb_write(circular_buffer_t* cb, const void* element) {
    if (cb->count == cb->capacity) {
        return false;
    }

    void *target = (char *)cb->data + cb->head * cb->element_size;
    memcpy(target, element, cb->element_size);

    cb->head = (cb->head + 1) % cb->capacity;
    cb->count++;
    
    return true;
}

bool cb_read(circular_buffer_t* cb, void* out) {
    if (cb->capacity == 0 || cb->count == 0) {
        return false;
    }

    void *source = (char *)cb->data + cb->tail * cb->element_size;
    memcpy(out, source, cb->element_size);

    cb->tail = (cb->tail + 1) % cb->capacity;
    cb->count--;

    return true;
}

void* cb_peek(circular_buffer_t* cb) {
    if (cb->capacity == 0 || cb->count == 0) {
        return NULL;
    }

    return (void*) ((char *)cb->data + cb->tail * cb->element_size);
}