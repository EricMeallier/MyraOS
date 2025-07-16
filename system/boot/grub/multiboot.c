#include <stdint.h>

#define MB_MAGIC   0x1BADB002u

#define MB_PAGE_ALIGN 1 << 0
#define MB_MEM_INFO   1 << 1
#define MBOOT_USE_GFX 0

#define MB_FLAGS (MB_PAGE_ALIGN | MB_MEM_INFO | MBOOT_USE_GFX)

#define MB_CHECKSUM -(MB_MAGIC + MB_FLAGS)

__attribute__((section(".multiboot.data"), used, aligned(4)))
static const uint32_t multiboot_header[3] = {
    MB_MAGIC,
    MB_FLAGS,
    MB_CHECKSUM
};