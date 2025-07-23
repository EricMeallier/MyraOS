#include <stdbool.h>

#include "interrupt/isr/isr.h"
#include "log/user_log.h"
#include "panic/panic.h"
#include "schedule/schedule.h"

#define RING_3 0x3

void isr_handler(const registers_t* regs) {
    const char* message = regs->int_no < ISR_EXCEPTION_AMOUNT 
        ? message = exception_messages[regs->int_no]
        : "Unknown Exception";

    if ((regs->cs & RING_3) == RING_3) {
        schedule_current_proc->state = PROCESS_TERMINATED;

        __asm__ volatile ("sti");
        ulog_errorf_nb("Process %d crashed with error: %s", schedule_current_proc->pid, message);
        __asm__ volatile ("cli");
        
        schedule_next();

        return;
    }

    kpanic(message);
}
