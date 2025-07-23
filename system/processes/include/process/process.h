#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>
#include <stddef.h>

#include "vmm/vmm.h"
#include "exec/exec.h"

#define PROCESS_STACK_START 0xBFFFFFFF
#define KERNEL_STACK_BASE 0xC0400000
#define KERNEL_STACK_SIZE 0x2000

#define PROCESS_HEAP_START 0x500000
#define PROCESS_HEAP_SIZE 0x100000

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

process_t* proc_create(exec_info_t* exec_info);

#endif // PROCESS_H