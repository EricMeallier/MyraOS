#include "interrupt/syscall/syscall.h"

#include <stddef.h>

#include "interrupt/syscall/syscall_numbers.h"

extern void _syscall_stub(void);

typedef uint32_t (*syscall_func_t)(uint32_t, uint32_t, uint32_t, uint32_t);

static syscall_func_t syscall_table[SYSCALL_MAX];

extern uint32_t sys_write(uint32_t, uint32_t, uint32_t, uint32_t);
extern uint32_t sys_read(uint32_t, uint32_t, uint32_t, uint32_t);

static void register_syscall(uint32_t index, syscall_func_t func) {
    if (index < SYSCALL_MAX) {
        syscall_table[index] = func;
    }
}

void syscall_init(void) {
    idt_set_gate(
        SYSCALL_INTERRUPT,
        (uint32_t) _syscall_stub,
        KERNEL_CODE_SEG,
        IDT_FLAG_PRESENT | IDT_FLAG_RING3 | IDT_FLAG_GATE_TASK_32BIT_INT
    );

    register_syscall(SYS_WRITE, sys_write);
    register_syscall(SYS_READ,  sys_read);
}

void syscall_handler(const registers_t* regs) {
    uint32_t func_num = regs->eax;

    if (func_num >= SYSCALL_MAX || syscall_table[func_num] == NULL) {
        return;
    }

    uint32_t result = syscall_table[func_num](regs->ebx, regs->ecx, regs->edx, regs->esi);

    ((registers_t*)regs)->eax = result;
}