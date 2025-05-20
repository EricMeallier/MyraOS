#include "interrupt/irq/irq.h"

void (*const irq_stubs[IRQ_SIZE])(void) = {
    _irq0, _irq1, _irq2, _irq3,
    _irq4, _irq5, _irq6, _irq7,
    _irq8, _irq9, _irq10, _irq11,
    _irq12, _irq13, _irq14, _irq15
};
