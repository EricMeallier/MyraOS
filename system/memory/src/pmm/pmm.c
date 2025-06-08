#include "pmm/pmm.h"

#include <stdbool.h>

volatile uint8_t bitmap[BITMAP_SIZE];

static inline void set_bit(const size_t bit);
static inline void clear_bit(const size_t bit);
static inline bool test_bit(const size_t bit);

static size_t free_page_count;

void pmm_init(void) {
    // init the bitmap
    for (size_t i = 0; i < BITMAP_SIZE; i++) {
        bitmap[i] = 0;
    }

    // mark the bios part as used
    for (size_t i = 0; i < BIOS_PART / BYTE_SIZE; i++) {
        bitmap[i] = 0xFF;
    }

    free_page_count = PAGES_AMOUNT - BIOS_PART;
}

void* pmm_alloc_page(void) {
    if (free_page_count == 0) {
        return NULL;
    }

    for (size_t i = BIOS_PART; i < BITMAP_SIZE; i++) {
        // check if at least 1 is free (set to 0)
        if (bitmap[i] < 0xFF) {
            for (size_t j = 0; j < BYTE_SIZE; j++) {
                size_t bit_index = i * BYTE_SIZE + j;

                // check if page is free
                if (!test_bit(bit_index)) {
                    set_bit(bit_index);
                    free_page_count--;

                    return (void *)((uintptr_t)bit_index * PAGE_SIZE);
                }
            }
        }
    }

    return NULL;
}

void pmm_free_page(void *addr) {
    uintptr_t bitmap_index = (uintptr_t)addr / PAGE_SIZE;

    if (bitmap_index >= PAGES_AMOUNT || bitmap_index < BIOS_PART) {
        return;
    }

    if (test_bit(bitmap_index)) {
        free_page_count++;
        clear_bit(bitmap_index);
    }
}

size_t pmm_get_free_page_amount(void) { return free_page_count; }

static inline void set_bit(const size_t bit) {
    size_t bitmap_index = bit / BYTE_SIZE;
    size_t bit_index = bit % BYTE_SIZE;

    uint8_t mask = 1 << bit_index;

    bitmap[bitmap_index] |= mask;
}

static inline void clear_bit(const size_t bit) {
    size_t bitmap_index = bit / BYTE_SIZE;
    size_t bit_index = bit % BYTE_SIZE;

    uint8_t mask = ~(1 << bit_index);

    bitmap[bitmap_index] &= mask;
}

static inline bool test_bit(const size_t bit) {
    size_t bitmap_index = bit / BYTE_SIZE;
    size_t bit_index = bit % BYTE_SIZE;

    return (bitmap[bitmap_index] >> bit_index) & 1;
}
