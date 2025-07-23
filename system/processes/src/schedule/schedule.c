#include "schedule/schedule.h"

#include "circular_buffer/circular_buffer.h"
#include "gdt/gdt.h"
#include "heap/heap.h"
#include "panic/panic.h"

static circular_buffer_t processes_buffer;
static bool is_schedule_ready;

process_t* schedule_current_proc;

extern void _switch_to_proc_space(uint32_t entry, uint32_t user_stack, uint32_t user_stack_top, uint32_t page_dir_phys);

void schedule_init(void) {
    cb_init(&processes_buffer, sizeof(process_t*), SCHEDULE_MAX_COUNT);
    is_schedule_ready = true;
}

void schedule_next(void) {
    if (!is_schedule_ready) {
        return;
    }

    process_t* next_proc = NULL;
    while (cb_read(&processes_buffer, &next_proc)) {
        if (next_proc->state != PROCESS_TERMINATED) {
            break;
        }
    }

    if (schedule_current_proc && schedule_current_proc->state != PROCESS_TERMINATED) {
        schedule_proc(schedule_current_proc);
    } else if (schedule_current_proc) {
        kfree(schedule_current_proc);
        schedule_current_proc = NULL;
    }

    if (!next_proc) {
        if (!schedule_current_proc) {
            kpanic("No process to run");
        } else {
            next_proc = schedule_current_proc;
        }
    }

    next_proc->state = PROCESS_RUNNING;
    schedule_current_proc = next_proc;

    tss_set_kernel_stack(next_proc->kernel_stack);
    
    _switch_to_proc_space(next_proc->regs->eip, next_proc->regs->esp, next_proc->regs->ebp, next_proc->page_directory);
}

void schedule_save_context(registers_t* regs) {
    if (!schedule_current_proc) {
        return;
    }

    task_state_t* state = schedule_current_proc->regs;
    state->eax = regs->eax;
    state->ebx = regs->ebx;
    state->ecx = regs->ecx;
    state->edx = regs->edx;
    state->esi = regs->esi;
    state->edi = regs->edi;
    state->ebp = regs->ebp;
    state->esp = regs->useresp;
    state->eip = regs->eip;
    state->eflags = regs->eflags;
    state->cs = regs->cs;
    state->ds = regs->ds;
    state->es = regs->es;
    state->fs = regs->fs;
    state->gs = regs->gs;
    state->ss = regs->ss;
}

bool schedule_proc(process_t* proc) {
    proc->state = PROCESS_READY;
    return cb_write(&processes_buffer, &proc);
}
