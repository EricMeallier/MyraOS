#ifndef LIBC_USER_STDLIB_H
#define LIBC_USER_STDLIB_H

#include <stddef.h>
#include <stdint.h>

void* malloc(size_t size);
void  free(void* ptr);
void* realloc(void* ptr, size_t size);

void userlib_heap_init(uintptr_t start, size_t size);

#endif // LIBC_USER_STDLIB_H
