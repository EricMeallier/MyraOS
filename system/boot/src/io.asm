%ifndef __IO_ASM__
%define __IO_ASM__

[bits 16]

global print
global clear_screen
global move_cursor

clear_screen:
    ; input: none
    ; modifies: ax, cx

    push ax
    push cx

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

    pop cx
    pop ax

    ret

print:
    ; input: si = message to print
    ; modifies: ax, bx, di

    push ax
    push bx
    push di

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

    .done:
        mov bx, di
        shr bx, 1
        call move_cursor

    pop di
    pop bx
    pop ax

    ret

move_cursor:
    ; input: bx = char index
    ; modifies: ax, dx

    push ax
    push dx

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

    pop dx
    pop ax

    ret

%endif