#include "interrupt/idt/idt.h"

#include "interrupt/isr/isr.h"

#include <stddef.h>

idt_entry_t idt[IDT_SIZE];
idt_descriptor_t idt_descriptor;

void idt_set_gate(uint8_t index, uint32_t base, uint16_t segment, uint8_t flags) {
    idt[index].offset_low = base & 0xFFFF;
    idt[index].segment_selector = segment;
    idt[index].reserved = 0;
    idt[index].type_attr = flags;
    idt[index].offset_high = (base >> 16) & 0xFFFF;
}

// implemented in idt_load.asm
extern void _idt_load(idt_descriptor_t* idt_descriptor);

void idt_init(void) {
    idt_descriptor.limit = IDT_ENTRY_SIZE * IDT_SIZE -1;
    idt_descriptor.idt = idt;

    for (size_t i = 0; i < ISR_EXCEPTION_AMOUNT; i++) {
        idt_set_gate(
            i,
            (uint32_t) interrupt_stubs[i],
            KERNEL_CODE_SEG,
            IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_TASK_32BIT_INT
        );
    }

    _idt_load(&idt_descriptor);
}
