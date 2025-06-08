#include <stdbool.h>

#include "interrupt/isr/isr.h"
#include "panic/kpanic.h"

void isr_handler(const registers_t* regs) {
    const char* message = regs->interrupt < ISR_EXCEPTION_AMOUNT 
        ? message = exception_messages[regs->interrupt]
        : "Unknown Exception";

    kpanic(message);
}
