#include "heap/heap.h"

#include "libc_kernel/string.h"
#include "assert/assert.h"
#include "pmm/pmm.h"

#define GET_METADATA_ADDR(block) ((void*) ((uintptr_t) block + sizeof(heap_block_t)))

static heap_block_t* split_block(heap_block_t* block, size_t size);

heap_block_t* heap_start;
heap_block_t* heap_end;

void heap_init(uintptr_t start, size_t size) {
    heap_start = (heap_block_t*) start;

    heap_start->size = size - sizeof(heap_block_t);
    heap_start->free = true;
    heap_start->next = NULL;

    heap_end = heap_start;
}

void* kmalloc(size_t size) {
    if (size == 0) {
        return NULL;
    }

    // align the size to fit 32 bits
    size = (size + 3) & ~3;

    heap_block_t* curr_block = heap_start;
    while (curr_block != NULL && (!curr_block->free || curr_block->size < size)) {
        curr_block = curr_block->next;
    }

    if (curr_block == NULL) {
        curr_block = (heap_block_t*) pmm_alloc_page();

        // page alloc failed
        if (curr_block == NULL) {
            return NULL;
        }
    
        // init the new block
        curr_block->size = PAGE_SIZE - sizeof(heap_block_t);
        curr_block->free = false;
        curr_block->next = NULL;

        // skip to the splitting logic
        heap_end->next = curr_block;
        heap_end = curr_block;
    } else if (curr_block->size == size) {
        curr_block->free = false;

        return GET_METADATA_ADDR(curr_block);
    }

    // split the block
    if (curr_block->size > size + sizeof(heap_block_t) + 8) {
        split_block(curr_block, size);
    }

    return GET_METADATA_ADDR(curr_block);
}

void kfree(void* ptr) {
    if (ptr == NULL) {
        return;
    }

    heap_block_t* target = (heap_block_t*)((uintptr_t)ptr - sizeof(heap_block_t));
    target->free = true;

    // merge free blocks
    heap_block_t* curr = heap_start;
    while (curr != NULL) {
        if (curr->free && curr->next != NULL && curr->next->free) {
            curr->size += sizeof(heap_block_t) + curr->next->size;
            curr->next = curr->next->next;

            if (curr->next == NULL) {
                heap_end = curr;
            }

            // continue checking for free blocks
            continue;
        }
        curr = curr->next;
    }
}

void* krealloc(void* ptr, size_t size) { 
    if (ptr == NULL) {
        return kmalloc(size);
    }

    if (size == 0) {
        kfree(ptr);
        return NULL;
    }

    // align the size to fit 32 bits
    size = (size + 3) & ~3;

    heap_block_t* block = (heap_block_t*)((uintptr_t)ptr - sizeof(heap_block_t));
    heap_block_t* curr = block;

    // split block into two smaller ones
    if (size < block->size) {
        block = split_block(block, size);
        
        return GET_METADATA_ADDR(block);
    }

    size_t blocks_size = 0;

    heap_block_t* next = block->next;
    blocks_size = block->size;

    while (next && next->free && blocks_size < size) {
        blocks_size += sizeof(heap_block_t) + next->size;
        next = next->next;
    }

    // allocate new block
    if (blocks_size < size) { 
        void* new_ptr = kmalloc(size);
        kmemcpy(new_ptr, ptr, block->size);
        kfree(ptr);

        return new_ptr;
    }

    // resize previous block
    block->size = blocks_size;
    block->next = curr->next;

    if (blocks_size == size) {
        block->free = false;
        return GET_METADATA_ADDR(block);
    }
    
    // split block
    if (blocks_size > size + sizeof(heap_block_t) + 8) {
        block = split_block(block, size);
    }
    
    return GET_METADATA_ADDR(block);
}

/*
    Function recieves block and the desired size for the block, and splits block into two.
*/
heap_block_t* split_block(heap_block_t* block, size_t size) {
    kassert(block->size > size);

    heap_block_t* new_block = (heap_block_t*) ((uintptr_t)block + sizeof(heap_block_t) + size);
    
    new_block->size = block->size - size - sizeof(heap_block_t);
    new_block->free = true;
    new_block->next = block->next;

    if (new_block->next == NULL) {
        heap_end = new_block;
    }

    block->size = size;
    block->free = false;
    block->next = new_block; 

    return block;
}
