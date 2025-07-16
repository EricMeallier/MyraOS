#ifndef MEM_CONSTANTS_H
#define MEM_CONSTANTS_H

#define TOTAL_MEMORY (1024 * 1024 * 1024)        // 1 GB
#define PAGE_SIZE 4096                           // 4 KB
#define PAGES_AMOUNT (TOTAL_MEMORY / PAGE_SIZE)  // 262,144

#define PAGE_ENTRIES 1024

#define BYTE_SIZE 8

#define KERNEL_PMA 0x100000
#define KERNEL_VMA 0xC0000000

#endif // MEM_CONSTANTS_H