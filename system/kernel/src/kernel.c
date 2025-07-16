#include <stdbool.h>

#include "gdt/gdt.h"
#include "heap/heap.h"
#include "interrupt/idt/idt.h"
#include "interrupt/irq/irq.h"
#include "interrupt/isr/isr.h"
#include "interrupt/syscall/syscall.h"
#include "keyboard/keyboard.h"
#include "block_device/pata.h"
#include "pit/pit.h"
#include "pmm/pmm.h"
#include "print/print.h"
#include "rtc/rtc.h"
#include "shell.h"
#include "vmm/vmm.h"

void kernel_main(void) {
    pmm_init();
    vmm_init();
    
    heap_init(HEAP_START_ADDR, HEAP_SIZE);

    gdt_init();

    idt_init();
    isr_init();
    irq_init();
    syscall_init();

    pit_init();
    rtc_init();

    keyboard_driver_install();

    pata_init();

    kprint("Welcome to ");
    kset_color(COLOR_GREEN);
    kprint("MyraOS");
    kset_color(COLOR_WHITE);
    kprintln(" v0.1");

    datetime_t dt = rtc_get_system_datetime();
    kprintf("Current Time: %02d:%02d:%02d, Date: %02d/%02d/%d (Weekday: %d)\n",
            dt.hour, dt.minute, dt.second,
            dt.day, dt.month, dt.year,
            dt.weekday
        );

    shell_run();

    while (true) {
        __asm__ volatile("hlt");
    }
}
