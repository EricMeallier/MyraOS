#include "process/process.h"

#include "gdt/gdt.h"
#include "heap/heap.h"
#include "libc_kernel/string.h"
#include "pmm/pmm.h"

#define PROCESS_USER_STACK 0xBFFFFFFF
#define PROCESS_KERNEL_STACK 0xC0400000

static size_t current_pid = 0;

static void copy_user_code(page_directory_t* page_dir_virt, uint32_t page_dir_phys, exec_info_t* exec_info) {
    // We have to remap here, keep it here for now 
    page_dir_virt->entries[PAGE_ENTRIES - 1] = (uint32_t) page_dir_phys | PAGE_PRESENT | PAGE_WRITE;

    uint32_t saved_cr3;
    __asm__ volatile("mov %%cr3, %0" : "=r"(saved_cr3));
    __asm__ volatile("cli");
    __asm__ volatile("mov %0, %%cr3" :: "r"(page_dir_phys));

    for (size_t i = 0; i < exec_info->segment_count; i++) {
        exec_segment_t* seg = &exec_info->segments[i];

        uint32_t virt_addr = seg->virt_addr;
        uint32_t start_page = PAGE_ALIGN_DOWN(virt_addr);
        uint32_t end_page = PAGE_ALIGN_UP(virt_addr + seg->mem_size);
        uint32_t num_pages = (end_page - start_page) / PAGE_SIZE;

        for (uint32_t j = 0; j < num_pages; j++) {
            uint32_t page_virt = start_page + j * PAGE_SIZE;
            uint32_t frame = (uint32_t) pmm_alloc_page();

            uint32_t page_flags = PAGE_PRESENT | PAGE_USER;
            if (seg->flags & SEGMENT_WRITE) {
                page_flags |= PAGE_WRITE;
            }

            vmm_map_page(page_virt, frame, page_flags);
        }

        kmemcpy((void*)virt_addr, seg->data, seg->file_size);

        if (seg->mem_size > seg->file_size) {
            kmemset((void*)(virt_addr + seg->file_size), 0, seg->mem_size - seg->file_size);
        }
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

process_t* proc_create(exec_info_t* exec_info) {
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
    copy_user_code(page_dir_virt, (uint32_t) page_dir_phys, exec_info);

    // Create and map the kernel stack
    uint32_t kernel_stack_phys = (uint32_t) pmm_alloc_page();
    uint32_t kernel_stack_virt = 0xC0400000 + current_pid * 0x2000;
    vmm_map_page(kernel_stack_virt - PAGE_SIZE, kernel_stack_phys, PAGE_PRESENT | PAGE_WRITE);
    map_kernel_stack_child((uint32_t) page_dir_phys, kernel_stack_virt, kernel_stack_phys);

    task_state_t* task_state = (task_state_t*) kmalloc(sizeof(task_state_t));
    kmemset(task_state, 0, sizeof(task_state_t));

    task_state->esp = PROCESS_USER_STACK;
    task_state->eip = (uint32_t) exec_info->entry_point;
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