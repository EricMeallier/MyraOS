#include "vmm/vmm.h"

#include "panic/kpanic.h"
#include "pmm/pmm.h"

#define MAKE_ENTRY(ptr, flags) (((uint32_t)(ptr) & 0xFFFFF000) | ((flags) & 0xFFF))
#define RECURSIVE_PAGE_TABLE_BASE 0xFFC00000

extern void load_page_directory(uint32_t dir);
extern void enable_paging(void);

static page_directory_t* kernel_page_directory;

void vmm_init(void) {
    kernel_page_directory = (page_directory_t*) pmm_alloc_page();
    page_table_t* first_table = (page_table_t*) pmm_alloc_page();

    for (size_t i = 0; i < PAGE_ENTRIES; i++) {
        kernel_page_directory->entries[i] = 0;
        first_table->entries[i] = 0;
    }

    // identity-map the first 4MB using 4KB pages
    for (size_t i = 0; i < PAGE_ENTRIES; i++) {
        uint32_t addr = i * PAGE_SIZE;
        first_table->entries[i] = MAKE_ENTRY(addr, PAGE_PRESENT | PAGE_WRITE); 
    }

    kernel_page_directory->entries[0] = MAKE_ENTRY(first_table, PAGE_PRESENT | PAGE_WRITE);
    kernel_page_directory->entries[PAGE_ENTRIES - 1] = MAKE_ENTRY(kernel_page_directory, PAGE_PRESENT | PAGE_WRITE);

    load_page_directory((uint32_t) kernel_page_directory);

    enable_paging();
}

void vmm_map_page(uint32_t virtual_addr, uint32_t physical_addr, uint32_t flags) {
    uint32_t dir_index = (virtual_addr >> 22) & 0x3FF;
    uint32_t table_index = (virtual_addr >> 12) & 0x3FF;

    // in case there is no page table
    if (kernel_page_directory->entries[dir_index] == 0) {
        page_table_t* new_page_table = (page_table_t*) pmm_alloc_page();
        if (new_page_table == NULL) {
            kpanic("Out of memory while creating page table");
        }

        for (size_t i = 0; i < PAGE_ENTRIES; i++) {
            new_page_table->entries[i] = 0;
        }   

        kernel_page_directory->entries[dir_index] = MAKE_ENTRY(new_page_table, PAGE_PRESENT | PAGE_WRITE);
    }

    page_table_t* page_table = (page_table_t*)(RECURSIVE_PAGE_TABLE_BASE + dir_index * PAGE_SIZE);
    page_table->entries[table_index] = MAKE_ENTRY(physical_addr, flags);

    __asm__ volatile("invlpg (%0)" :: "r" (virtual_addr) : "memory");
}

void vmm_unmap_page(uint32_t virtual_addr) {
    uint32_t dir_index = (virtual_addr >> 22) & 0x3FF;
    uint32_t table_index = (virtual_addr >> 12) & 0x3FF;

    page_table_t* page_table = (page_table_t*)(RECURSIVE_PAGE_TABLE_BASE + dir_index * PAGE_SIZE);
    page_table->entries[table_index] = 0;

    __asm__ volatile("invlpg (%0)" :: "r" (virtual_addr) : "memory");
}
