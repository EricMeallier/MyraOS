#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef struct heap_block {
    size_t size;
    int    free;
    struct heap_block* next;
} heap_block_t;

#define ALIGN4(x) (((x) + 3) & ~3)
#define METADATA_SIZE sizeof(heap_block_t)

static heap_block_t* heap_start = NULL;
static heap_block_t* heap_end   = NULL;

void userlib_heap_init(uintptr_t start, size_t size) {
    heap_start = (heap_block_t*) start;
    heap_start->size = size - METADATA_SIZE;
    heap_start->free = 1;
    heap_start->next = NULL;
    heap_end = heap_start;
}

static void* get_block_data(heap_block_t* block) {
    return (void*) ((uintptr_t) block + METADATA_SIZE);
}

static heap_block_t* find_fit(size_t size) {
    heap_block_t* curr = heap_start;
    while (curr) {
        if (curr->free && curr->size >= size) return curr;
        curr = curr->next;
    }

    return NULL;
}

static void split_block(heap_block_t* block, size_t size) {
    if (block->size <= size + METADATA_SIZE + 8) return;

    heap_block_t* new_block = (heap_block_t*) ((uintptr_t) block + METADATA_SIZE + size);

    new_block->size = block->size - size - METADATA_SIZE;
    new_block->free = 1;
    new_block->next = block->next;

    block->size = size;
    block->next = new_block;

    if (heap_end == block) {
        heap_end = new_block;
    }
}

void* malloc(size_t size) {
    if (!size) {
        return NULL;
    }
    size = ALIGN4(size);

    heap_block_t* block = find_fit(size);
    if (!block) {
        return NULL;
    }

    block->free = 0;
    split_block(block, size);

    return get_block_data(block);
}

void free(void* ptr) {
    if (!ptr) {
        return;
    }

    heap_block_t* block = (heap_block_t*)((uintptr_t)ptr - METADATA_SIZE);
    block->free = 1;

    heap_block_t* curr = heap_start;
    while (curr && curr->next) {
        if (curr->free && curr->next->free) {
            curr->size += METADATA_SIZE + curr->next->size;
            curr->next = curr->next->next;
            continue;
        }
        curr = curr->next;
    }

    if (!curr->next) {
        heap_end = curr;
    }
}

void* realloc(void* ptr, size_t size) {
    if (!ptr) {
        return malloc(size);
    }

    if (!size) {
        free(ptr);
        return NULL;
    }

    heap_block_t* block = (heap_block_t*)((uintptr_t)ptr - METADATA_SIZE);
    if (block->size >= size) {
        return ptr;
    }

    void* new_ptr = malloc(size);
    if (!new_ptr) {
        return NULL;
    }

    memcpy(new_ptr, ptr, block->size);
    free(ptr);

    return new_ptr;
}
