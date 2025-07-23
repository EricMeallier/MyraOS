bits 32

%define GDT_USER_CODE 0x1B
%define GDT_USER_DATA 0x23

; void _switch_to_proc_space(uint32_t entry, uint32_t user_stack, uint32_t user_stack_top, uint32_t page_dir_phys)
global _switch_to_proc_space
_switch_to_proc_space:
    cli

    ; No need to create stack frame as we have no return
    mov ebx, [esp + 4]
    mov ecx, [esp + 8]
    mov edx, [esp + 12]
    mov esi, [esp + 16]

    mov cr3, esi

    xor eax, eax
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

    ; iret sets esp, we also need to set ebp
    mov ebp, edx

    sti
    iretd                      ; pop & jump to ring‑3


; Working:

; esp            0xbffffffb          0xbffffffb
; ebp            0xc0122cd8          0xc0122cd8

; esp            0xbfffffff          0xbfffffff
; ebp            0xc03fff88          0xc03fff88

; -----------

; Faulty:

; esp            0xbfffffeb          0xbfffffeb
; ebp            0xc03fdf88          0xc03fdf88

; But still both accessible:

; (gdb) x/x 0xc03fdf88
; 0xc03fdf88:     0xc03fdfa8
; (gdb) x/x 0xbfffffeb
; 0xbfffffeb:     0x00000000

; code:
;    0x400000:    push   %ebp
;    0x400001:    mov    %esp,%ebp
;    0x400003:    push   %ebx
;    0x400004:    sub    $0x10,%esp
;    0x400007:    mov    0x8(%ebp),%eax
;    0x40000a:    mov    0xc(%ebp),%ebx
;    0x40000d:    mov    0x10(%ebp),%ecx
;    0x400010:    mov    0x14(%ebp),%edx
;    0x400013:    int    $0x80
;    0x400015:    mov    %eax,-0x8(%ebp)
;    0x400018:    mov    -0x8(%ebp),%eax
;    0x40001b:    mov    -0x4(%ebp),%ebx
;    0x40001e:    leave
;    0x40001f:    ret
;    0x400020:    push   %ebp
;    0x400021:    mov    %esp,%ebp
;    0x400023:    mov    0x8(%ebp),%eax
;    0x400026:    push   0xc(%ebp)
;    0x400029:    push   %eax
;    0x40002a:    push   $0x1
;    0x40002c:    push   $0x4
;    0x40002e:    call   0x400000
;    0x400033:    add    $0x10,%esp
;    0x400036:    nop
;    0x400037:    leave
;    0x400038:    ret
;    0x400039:    push   %ebp
;    0x40003a:    mov    %esp,%ebp
;    0x40003c:    sub    $0x10,%esp
;    0x40003f:    movl   $0x0,-0x4(%ebp)
;    0x400046:    jmp    0x40004f
; => 0x400048:    mov    -0x4(%ebp),%eax             ; <------ fault here
;    0x40004b:    inc    %eax
;    0x40004c:    mov    %eax,-0x4(%ebp)
;    0x40004f:    mov    -0x4(%ebp),%eax
;    0x400052:    cmp    $0x98967f,%eax
;    0x400057:    jle    0x400048
;    0x400059:    push   $0x13
;    0x40005b:    push   $0x40006a
;    0x400060:    call   0x400020
;    0x400065:    add    $0x8,%esp
;    0x400068:    jmp    0x40003f
;    0x40006a:    dec    %eax
;    0x40006b:    gs insb (%dx),%es:(%edi)
;    0x40006d:    insb   (%dx),%es:(%edi)
;    0x40006e:    outsl  %ds:(%esi),(%dx)
;    0x40006f:    and    %ah,0x72(%esi)
;    0x400072:    outsl  %ds:(%esi),(%dx)
;    0x400073:    insl   (%dx),%es:(%edi)
;    0x400074:    and    %dh,0x72(%eax)
;    0x400077:    outsl  %ds:(%esi),(%dx)
;    0x400078:    arpl   %esp,(%eax)
;    0x40007a:    popa
;    0x40007b:    and    %ecx,(%edx)
;    0x40007d:    add    %al,(%eax)
;    0x40007f:    add    %al,(%eax)
;    0x400081:    add    %al,(%eax)
;    0x400083:    add    %al,(%eax)

; Both are mapped:

; (gdb) x/x $ebp
; 0xc03fdf88:     0xc03fdfa8
; (gdb) x/x $ebp-0x4
; 0xc03fdf84:     0x00000000

; Manually works:
; (gdb) set $eax = *((int*)($ebp - 4))
; (gdb) p/x $eax
; $2 = 0x0

; But still #PF at the same insn: 
; (gdb) si
; Breakpoint 1, 0xc0104fd4 in _isr14 ()