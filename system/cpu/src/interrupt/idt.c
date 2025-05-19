#include "interrupt/idt.h"

#include <stddef.h>

idt_entry_t idt[IDT_SIZE];
idt_descriptor_t idt_descriptor;

void idt_set_gate(uint8_t index, uint32_t base, uint16_t segment, uint8_t flags) {
    idt[index].offset_low = base & 0xFFFF;
    idt[index].segment_selector = segment;
    idt[index].reserved = 0x0000;
    idt[index].type_attr = flags;
    idt[index].offset_high = (base << 16) & 0xFFFF;
}