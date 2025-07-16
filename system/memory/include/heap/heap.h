#ifndef HEAP_H
#define HEAP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "constants/mem_constants.h"

extern uint8_t _kernel_end;
#define HEAP_START_ADDR (((uintptr_t)&_kernel_end + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))
#define HEAP_SIZE       0x400000  // 4MB heap

typedef struct heap_block_t {
    uint32_t size;
    bool free;
    struct heap_block_t* next;
} heap_block_t;

// init
void heap_init(uintptr_t start, size_t size);

// malloc & free
void* kmalloc(size_t size);

void kfree(void* ptr);

void* krealloc(void* ptr, size_t size);

#endif // HEAP_H