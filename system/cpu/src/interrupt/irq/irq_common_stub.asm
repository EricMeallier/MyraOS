[bits 32]

extern irq_handler

KERNEL_DATA_SEG equ 0x10

global _irq_common_stub

_irq_common_stub:
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

    call irq_handler
    add esp, 4

    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popa
    add esp, 4
    sti

    iret

    