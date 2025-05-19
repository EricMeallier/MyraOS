//
// Created by Dvir Biton on 15/05/2025.
//

#ifndef IDT_H
#define IDT_H

#include <stdint.h>

#define IDT_SIZE 256
#define IDT_ENTRY_SIZE sizeof(idt_entry_t)

typedef struct idt_entry_t {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t zero_unused;
    uint8_t type_attr;
    uint16_t offset_high;
} __attribute__((packed)) idt_entry_t;

typedef struct idt_ptr_t {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) idt_ptr_t;

#endif //IDT_H
