bits 32

%define GDT_USER_CODE 0x1B
%define GDT_USER_DATA 0x23

global switch_to_user_mode
switch_to_user_mode:
    cli

    mov ax, GDT_USER_DATA
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax          ; (optional)

    ; stack for iret:  SS, ESP, EFLAGS, CS, EIP
    push dword GDT_USER_DATA   ; SS  (ring‑3)
    push 0xBFFFFFFF            ; ESP
    push dword 0x202           ; EFLAGS (IF=1)
    push dword GDT_USER_CODE   ; CS  (ring‑3)
    push 0x400000              ; EIP

    iretd                      ; pop & jump to ring‑3
