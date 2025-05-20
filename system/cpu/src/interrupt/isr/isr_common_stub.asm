[bits 32]

KERNEL_DATA_SEG equ 0x10

extern isr_handler

global _isr_common_stub
_isr_common_stub:
    pusha
    push ds
    push es
    push fs
    push gs

    mov ax, KERNEL_DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    call isr_handler

    pop gs
    pop fs
    pop es
    pop ds
    popa

    ; clean the error and the interrupt
    add esp, 8
    sti

    iret
    