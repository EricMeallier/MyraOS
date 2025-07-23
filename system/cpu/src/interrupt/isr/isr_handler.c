#include <stdbool.h>

#include "interrupt/isr/isr.h"
#include "panic/panic.h"

void isr_handler(const registers_t* regs) {
    const char* message = regs->int_no < ISR_EXCEPTION_AMOUNT 
        ? message = exception_messages[regs->int_no]
        : "Unknown Exception";

    kpanic(message);
}
