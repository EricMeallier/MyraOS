//
// Created by dvir on 5/20/25.
//

#ifndef ISR_H
#define ISR_H

#include <stdint.h>

#define ISR_EXCEPTION_AMOUNT 32

typedef struct {
    uint32_t ds; // manual
    uint32_t edi, esi, ebp, useless, ebx, edx, ecx, eax; // pusha
    uint32_t interrupt, error; // manual/automatic
    uint32_t eip, cs, eflags, esp, ss; // CPU pushed
} __attribute__((packed)) registers_t;

// stubs
extern void (*const interrupt_stubs[ISR_EXCEPTION_AMOUNT])(void);

// handler
extern void isr_handler(const registers_t* regs);

// error messages
extern const char *exception_messages[ISR_EXCEPTION_AMOUNT];

// isr
extern void _isr0();
extern void _isr1();
extern void _isr2();
extern void _isr3();
extern void _isr4();
extern void _isr5();
extern void _isr6();
extern void _isr7();
extern void _isr8();
extern void _isr9();
extern void _isr10();
extern void _isr11();
extern void _isr12();
extern void _isr13();
extern void _isr14();
extern void _isr15();
extern void _isr16();
extern void _isr17();
extern void _isr18();
extern void _isr19();
extern void _isr20();
extern void _isr21();
extern void _isr22();
extern void _isr23();
extern void _isr24();
extern void _isr25();
extern void _isr26();
extern void _isr27();
extern void _isr28();
extern void _isr29();
extern void _isr30();
extern void _isr31();

#endif // ISR_H