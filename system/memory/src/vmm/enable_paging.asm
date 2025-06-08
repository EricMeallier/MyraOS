[bits 32]

global enable_paging
global disable_paging

enable_paging:
    push ebp
    mov ebp, esp

    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax

    mov esp, ebp,
    pop ebp

    ret

disable_paging:
    mov eax, cr0
    and eax, 0x7FFFFFFF
    mov cr0, eax

    ret