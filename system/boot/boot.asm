[org 0x7C00]
[bits 16]

KERNEL_CODE_SEG equ 0x08
KERNEL_DATA_SEG equ 0x10
USER_CODE_SEG equ 0x18
USER_DATA_SEG equ 0x20

KERNEL_LOAD_SEG equ 0x1000 
KERNEL_START_ADDR equ 0x10000

KERNEL_SEGMENTS equ 64

start:
    jmp main

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

    call .load_kernel

    mov si, welcome

    call print

.load_kernel:
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

clear_screen:
    ; input: none
    ; modifies: ax, cx

    mov ax, 0xB800
    mov es, ax
    xor di, di

    mov al, ' '
    mov ah, 0x0F

    mov cx, 80 * 25

    .clear_cells:
        mov [es:di], ax

        add di, 2
        loop .clear_cells

        ret

print:
    ; input: si = message to print
    ; modifies: ax

    mov ax, 0xB800
    mov es, ax
    xor di, di

    mov ah, 0x0F

    .next_char:
        lodsb

        cmp al, 0
        je .done

        mov [es:di], ax
        
        add di, 2
        jmp .next_char
    
    ret

    .done:
        ; input: none
        ; modifies: bx

        mov bx, di
        shr bx, 1
        call move_cursor

        call enable_a20

        call set_gdt

        jmp enable_protected_mode

move_cursor:
    ; input: bx = char index
    ; modifies: al, dx

    mov dx, 0x3D4
    mov al, 0x0F
    out dx, al

    inc dx
    mov al, bl
    out dx, al

    dec dx
    mov al, 0x0E
    out dx, al

    inc dx
    mov al, bh
    out dx, al

    ret

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

    ; tss 
    ; would be added later
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

welcome: db 'Welcome to MyraOS! Booting...', 0
kernel_injection_error: db 'Error: Could not inject kernel', 0


[bits 32]

protected_mode_start:
    mov ax, KERNEL_DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov esp, 0x90000

    jmp KERNEL_CODE_SEG:KERNEL_START_ADDR

times 510 - ($ - $$) db 0
dw 0xAA55
