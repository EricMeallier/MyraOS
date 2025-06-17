#include "pit/pit.h"

#include "io/port_io.h"
#include "irq/irq.h"

#define PIT_CHANNEL_0_DATA_PORT 0x40
#define PIT_CONTROL_PORT 0x43

static uint64_t tick_count = 0;

void pit_init(void) {
    uint16_t divisor = PIT_BASE_HZ / PIT_HZ;

    outb(PIT_CONTROL_PORT, 0x36);
    outb(PIT_CHANNEL_0_DATA_PORT, divisor & 0xFF);       
    outb(PIT_CHANNEL_0_DATA_PORT, (divisor >> 8) & 0xFF); 

    irq_install_handler(PIT_IRQ, pit_handler);
}

void pit_handler(registers_t* regs) {
    tick_count++;
}

uint64_t pit_ticks(void) {
    return tick_count;
}