[org 0x7C00]
[bits 16]

start:
    jmp main

KERNEL_CODE_SEG equ 0x08
KERNEL_DATA_SEG equ 0x10
USER_CODE_SEG equ 0x18
USER_DATA_SEG equ 0x20
TSS_SEG equ 0x28

KERNEL_LOAD_SEG equ 0x1000 
KERNEL_START_ADDR equ 0x10000

KERNEL_SEGMENTS equ 64

welcome: db 'Welcome to MyraOS! Booting...', 0
kernel_injection_error: db 'Error: Could not inject kernel', 0

main:
    cli

    call clear_screen

    ; setup data segments
    mov ax, 0x00
    
    mov ds, ax
    mov es, ax

    ; setup stack
    mov ss, ax
    mov sp, 0x7C00

    sti

    call load_kernel

    mov si, welcome

    call print

    jmp setup

%include "system/boot/src/io.asm"

load_kernel:
    ; input: none
    ; modifies: ax, bx, cx, dx, es

    mov ax, KERNEL_LOAD_SEG
    mov es, ax
    xor bx, bx
    mov dh, 0x00
    mov dl, 0x00
    mov cl, 0x02
    mov ch, 0x00
    mov ah, 0x02
    mov al, KERNEL_SEGMENTS
    int 0x13

    jc disk_read_error

    ret

disk_read_error:
    ; input: none
    ; modifies: si

    mov si, kernel_injection_error

    call print

    hlt

setup:
    ; input: none
    ; modifies: bx

    call enable_a20

    call set_gdt

    jmp enable_protected_mode

enable_a20:
    ; input: none
    ; modifies: al

    in al, 0x92
    or al, 00000010b
    out 0x92, al

    ret

gdt_start:
    ; input: none
    ; modifies: none

    ; null descriptor
    dq 0

    ; kernel mode - code segment: Base = 0, Limit = 0xFFFFF, Access Byte = 0x9A, Flags = 0xC
    dw 0xFFFF      ; limit
    dw 0x0000      ; base low
    db 0x00        ; base mid
    db 0x9A        ; access
    db 0xCF        ; flags
    db 0x00        ; base high

    ; kernel mode - data segment: Base = 0, Limit = 0xFFFFF, Access Byte = 0x92, Flags = 0xC
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 0x92
    db 0xCF
    db 0x00

    ; user mode - code segment: Base = 0, Limit = 0xFFFFF, Access Byte = 0xFA, Flags = 0xC
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 0xFA
    db 0xCF
    db 0x00

    ; user mode - data segment: Base = 0, Limit = 0xFFFFF, Access Byte = 0xF2, Flags = 0xC
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 0xF2
    db 0xCF
    db 0x00

    ; TSS descriptor: place holder
    gdt_tss:
    dw 0x00            
    dw 0x00                           
    db 0x00
    db 0x00                        
    db 0x00                        
    db 0x00
gdt_end:

gdt_descriptor:
    ; input: none
    ; modifies: none

    dw gdt_end - gdt_start - 1   ; limit
    dd gdt_start                 ; base address

set_gdt:
    ; input: none
    ; modifies: eax

    cli
    lgdt [gdt_descriptor]
    sti

    ret

enable_protected_mode:
    ; input: none
    ; modifies: eax

    cli
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    jmp KERNEL_CODE_SEG:protected_mode_start

[bits 32]

tss:
    dw 0 ; prev task link
    dw 0 
    dd 0 ; esp0 
    dw KERNEL_DATA_SEG
    dw 0
    times 23 dd 0 ; padding to 104 bytes
tss_end:

protected_mode_start:
    mov ax, KERNEL_DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; --- build the TSS descriptor ---
    mov ebx, gdt_tss          ; EBX = address of placeholder
    mov eax, tss              ; EAX = base of the TSS

    ; limit (103 = 0x67)
    mov word [ebx], tss_end - tss - 1

    ; base 0-15
    mov word [ebx + 2], ax

    ; base 16-23
    shr eax, 16
    mov byte [ebx + 4], al

    ; access byte: 0x89  (Present=1, DPL=0, type=9 = Available 32-bit TSS)
    mov byte [ebx + 5], 0x89

    ; flags / limit high: G=0, D/B=0, L=0, AVL=0, limit(19..16)=0
    mov byte [ebx + 6], 0

    ; base 24-31
    mov byte [ebx + 7], ah

    ; --- store esp0/ss0 in the TSS ---
    mov dword [tss + 4], 0x9FFFC     ; esp0  (your kernel stack top)
    mov  word [tss + 8], KERNEL_DATA_SEG

    ; --- load TR ---
    mov ax, TSS_SEG
    ltr ax

    jmp KERNEL_CODE_SEG:KERNEL_START_ADDR

times 510 - ($ - $$) db 0
dw 0xAA55
