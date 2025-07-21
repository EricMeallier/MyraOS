bits 32

%define GDT_USER_CODE 0x1B
%define GDT_USER_DATA 0x23

; void _switch_to_user_mode(uint32_t entry, uint32_t user_stack)
global _switch_to_user_mode
_switch_to_user_mode:
    cli

    ; No need to create stack frame as we have no return
    mov ebx, [esp + 4]
    mov ecx, [esp + 8]

    mov ax, GDT_USER_DATA
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax     

    ; stack for iret: SS, ESP, EFLAGS, CS, EIP
    push dword GDT_USER_DATA   ; SS  (ring‑3)
    push ecx                   ; ESP
    push dword 0x202           ; EFLAGS (IF=1)
    push dword GDT_USER_CODE   ; CS  (ring‑3)
    push ebx                   ; EIP

    iretd                      ; pop & jump to ring‑3
