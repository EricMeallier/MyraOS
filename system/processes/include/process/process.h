#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>
#include <stddef.h>

#include "vmm/vmm.h"

typedef struct task_state_t {
    uint32_t eax, ebx, ecx, edx;
    uint32_t esi, edi, ebp, esp;
    uint32_t eip;
    uint32_t eflags;
    uint16_t cs, ds, es, fs, gs, ss;
} __attribute__((packed, aligned(4))) task_state_t;

typedef enum process_state_t {
    PROCESS_NEW,
    PROCESS_RUNNING,
    PROCESS_READY,
    PROCESS_WAITING,
    PROCESS_TERMINATED,
} process_state_t;

typedef struct process_t {
    uint32_t pid;
    process_state_t state;
    uint32_t page_directory;
    task_state_t* regs;
    uint32_t kernel_stack;
} process_t;

process_t* proc_create(const void* entry, size_t code_size, uint32_t code_addr);

#endif // PROCESS_H