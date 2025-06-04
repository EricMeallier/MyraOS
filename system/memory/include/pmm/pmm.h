#ifndef PMM_H
#define PMM_H

#include <stdint.h>
#include <stddef.h>

#define TOTAL_MEMORY (1024 * 1024 * 1024) // 1 GB
#define PAGE_SIZE 4096 // 4 KB
#define PAGES_AMOUNT (TOTAL_MEMORY / PAGE_SIZE) // 262,144

#define BITMAP_SIZE PAGES_AMOUNT / 8

#define BIOS_PART 512

extern volatile uint8_t bitmap[BITMAP_SIZE];

// init
void pmm_init(void);

// allocate
void* pmm_alloc_page(void);

// free
void pmm_free_page(void* addr);

// get free amount
size_t pmm_get_free_page_amount(void);

#endif // PMM_H