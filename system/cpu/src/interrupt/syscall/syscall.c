#include "interrupt/syscall/syscall.h"

#include <stddef.h>

#include "print/print.h"

extern void _syscall_stub(void);

typedef uint32_t (*syscall_func_t)(uint32_t, uint32_t, uint32_t, uint32_t);

static syscall_func_t syscall_table[SYSCALL_MAX];

static uint32_t sys_test(uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
    (void) a; (void) b; (void) c; (void) d;
    
    kprintf("Hello, world! - From kernel!");

    return 42;
}

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

    register_syscall(0, sys_test);
}

void syscall_handler(const registers_t* regs) {
    uint32_t func_num = regs->eax;

    if (func_num >= SYSCALL_MAX || syscall_table[func_num] == NULL) {
        return;
    }

    uint32_t result = syscall_table[func_num](regs->ebx, regs->ecx, regs->edx, regs->esi);

    ((registers_t*)regs)->eax = result;
}