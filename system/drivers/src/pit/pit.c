#include "pit/pit.h"

#include "frame/frame.h"
#include "io/port_io.h"
#include "interrupt/irq/irq.h"
#include "input/input.h"
#include "schedule/schedule.h"

#define PIT_CHANNEL_0_DATA_PORT 0x40
#define PIT_CONTROL_PORT 0x43

#define RING_3 0x3

static uint32_t tick_count = 0;

void pit_init(void) {
    uint16_t divisor = PIT_BASE_HZ / PIT_HZ;

    outb(PIT_CONTROL_PORT, 0x36);
    outb(PIT_CHANNEL_0_DATA_PORT, divisor & 0xFF);       
    outb(PIT_CHANNEL_0_DATA_PORT, (divisor >> 8) & 0xFF); 

    irq_install_handler(PIT_IRQ, pit_handler);
}

void pit_handler(registers_t* regs) {
    tick_count++;

    outb(0x20, 0x20);

    if (tick_count % 10 == 0) {
        input_process();

        frame_render();

        if ((regs->cs & RING_3) == RING_3) {
            schedule_save_context(regs);
            schedule_next();
        }
    }
}

uint32_t pit_ticks(void) {
    return tick_count;
}