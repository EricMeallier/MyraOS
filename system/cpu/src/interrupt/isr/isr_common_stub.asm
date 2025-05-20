[bits 32]

KERNEL_DATA_SEG equ 0x10

extern isr_handler

global _isr_common_stub
_isr_common_stub:
    pusha

    xor eax, eax
    mov ax, ds
    push eax

    mov ax, KERNEL_DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov eax, esp
    push eax

    call isr_handler
    add esp, 4

    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popa

    ; clean the error and the interrupt
    add esp, 8
    sti

    iret
