#include "interrupt/irq/irq.h"
#include "interrupt/idt/idt.h"
#include "print/kprint.h"
#include "io/port_io.h"

#include <stddef.h>

void irq_install_handler(int irq, void (*handler)(registers_t* regs)) {
    irq_routines[irq] = handler;
}

void irq_handler(registers_t* regs) {
    int irq = regs->interrupt - IRQ_INTERRUPT_OFFSET;

    kprint("IRQ error");

    if (irq < IRQ_SIZE && irq_routines[irq]) {
        irq_routines[irq](regs);
    }

    if (irq > 7) 
        outb(0xA0, 0x20); // EOI to slave PIC
    outb(0x20, 0x20);     // EOI to master PIC

} 

void irq_remap() {
    outb(0x20, 0x11);  // init PIC1 & PIC2
    outb(0xA0, 0x11);

    outb(0x21, IRQ_INTERRUPT_OFFSET); // remap offset irq 0-7 to idt 32-39
    outb(0xA1, IRQ_INTERRUPT_OFFSET + 8); // remap offset irq 8-15 to idt 40-47

    outb(0x21, 0x04); // ICW3: tell master PIC that slave is at IRQ2
    outb(0xA1, 0x02); // ICW3: tell slave PIC its cascade identity

    outb(0x21, 0x01); // 8086 mode
    outb(0xA1, 0x01); 

    outb(0x21, 0x00); // unmask all interrupts
    outb(0xA1, 0x00); 
}

void install_irq() {
    irq_remap();

    for (size_t i = IRQ_INTERRUPT_OFFSET; i < IRQ_SIZE + IRQ_INTERRUPT_OFFSET; i++) {
        idt_set_gate(
            i,
            (uint32_t)irq_stubs[i - IRQ_INTERRUPT_OFFSET],
            KERNEL_CODE_SEG,
            IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_GATE_TASK_32BIT_INT
        );
    }
}