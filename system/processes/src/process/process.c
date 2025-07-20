#include "process/process.h"

#include "pmm/pmm.h"
#include "heap/heap.h"
#include "libc_kernel/string.h"
#include "gdt/gdt.h"

#define PROCESS_USER_STACK 0xBFFFFFFF
#define PROCESS_CODE 0x00400000

static size_t current_pid = 0;

static void copy_user_code(page_directory_t* page_dir_virt, uint32_t page_dir_phys, const void* entry_code, size_t code_size) {
    page_dir_virt->entries[PAGE_ENTRIES - 1] = (uint32_t) page_dir_phys | PAGE_PRESENT | PAGE_WRITE;

    uint32_t saved_cr3;
    __asm__ volatile("mov %%cr3, %0" : "=r"(saved_cr3));
    __asm__ volatile("cli");
    __asm__ volatile("mov %0, %%cr3" :: "r"(page_dir_phys));

    for (size_t offset = 0; offset < code_size; offset += PAGE_SIZE) {
        uint32_t* frame = (uint32_t*) pmm_alloc_page();
        vmm_map_page(PROCESS_CODE + offset, (uint32_t) frame, PAGE_PRESENT | PAGE_WRITE | PAGE_USER);

        size_t copy_size = code_size - offset > PAGE_SIZE ? PAGE_SIZE : code_size - offset;
        kmemcpy((void*)(PROCESS_CODE + offset), (const uint8_t*)entry_code + offset, copy_size);

        vmm_map_page(PROCESS_CODE + offset, (uint32_t) frame, PAGE_PRESENT | PAGE_USER);
    }

    __asm__ volatile("mov %0, %%cr3" :: "r"(saved_cr3));
    __asm__ volatile("sti");
}

static void map_kernel_stack_child(uint32_t page_dir_phys, uint32_t kernel_stack_virt, uint32_t kernel_stack_phys) {
    uint32_t saved_cr3;
    __asm__ volatile("mov %%cr3, %0" : "=r"(saved_cr3));
    __asm__ volatile("cli");
    __asm__ volatile("mov %0, %%cr3" :: "r"(page_dir_phys));

    vmm_map_page(kernel_stack_virt - PAGE_SIZE, kernel_stack_phys, PAGE_PRESENT | PAGE_WRITE);

    __asm__ volatile("mov %0, %%cr3" :: "r"(saved_cr3));
    __asm__ volatile("sti");
}

process_t* process_create(const void* entry, size_t code_size) {
    __asm__ volatile("cli");

    process_t* process = (process_t*) kmalloc(sizeof(process_t));
    kmemset(process, 0, sizeof(process_t));
    
    page_directory_t* page_dir_phys = (page_directory_t*) pmm_alloc_page();
    page_directory_t* page_dir_virt = page_dir_phys;
    
    vmm_map_page((uint32_t) page_dir_virt, (uint32_t) page_dir_phys, PAGE_PRESENT | PAGE_WRITE);
    page_dir_virt->entries[PAGE_ENTRIES - 1] = (uint32_t) page_dir_phys | PAGE_PRESENT | PAGE_WRITE;

    kmemset(page_dir_virt, 0, PAGE_SIZE);

    // Copy the kernel to the page dir of the proc
    kmemcpy(&page_dir_virt->entries[KERNEL_ENTRY_START], &current_page_directory->entries[KERNEL_ENTRY_START], (PAGE_ENTRIES - KERNEL_ENTRY_START) * sizeof(uint32_t));
    copy_user_code(page_dir_virt, (uint32_t) page_dir_phys, entry, code_size);

    // Create and map the kernel stack
    uint32_t kernel_stack_phys = (uint32_t) pmm_alloc_page();
    uint32_t kernel_stack_virt = 0xC0400000 + current_pid * 0x2000;
    vmm_map_page(kernel_stack_virt - PAGE_SIZE, kernel_stack_phys, PAGE_PRESENT | PAGE_WRITE);
    map_kernel_stack_child((uint32_t) page_dir_phys, kernel_stack_virt, kernel_stack_phys);

    task_state_t* task_state = (task_state_t*) kmalloc(sizeof(task_state_t));
    kmemset(task_state, 0, sizeof(task_state_t));

    task_state->esp = PROCESS_USER_STACK;
    task_state->eip = PROCESS_CODE;
    task_state->cs = GDT_USER_CODE;
    task_state->ds =
    task_state->es =
    task_state->fs =
    task_state->gs =
    task_state->ss = GDT_USER_DATA;
    task_state->eflags = 0x202; // IF = 1
        
    process->pid = current_pid++;
    process->state = PROCESS_NEW;
    process->page_directory = (uint32_t) page_dir_phys;
    process->regs = task_state;
    process->kernel_stack = kernel_stack_virt;
    
    __asm__ volatile("sti");

    return process;
}