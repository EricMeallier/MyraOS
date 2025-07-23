.section .text
.global _start
.extern userlib_heap_init
.extern main
.extern _exit

_start:
    # Make sure it matches the mapped pages in /system/processes/src/process/process.c
    push $0x100000
    push $0x500000
    call userlib_heap_init

    call main
    call _exit