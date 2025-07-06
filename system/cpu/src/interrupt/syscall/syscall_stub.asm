[bits 32]

KERNEL_DATA_SEG equ 0x10

extern syscall_handler

global _syscall_stub
_syscall_stub:
    cli
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

    call syscall_handler
    add esp, 4

    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popa 
    sti
    iret
    