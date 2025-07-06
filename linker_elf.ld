ENTRY(_start)
OUTPUT_FORMAT("elf32-i386")

SECTIONS {
    . = 0x10000;
    
    .entry : {
        *(.entry)
    }
    
    .text : {
        KEEP(*(.text._start))
        *(.text*)
    }
    
    .rodata : {
        *(.rodata*)
    }
    
    .data : {
        *(.data*)
    }
    
    .bss : {
        *(COMMON)
        *(.bss*)
    }
    
    . = ALIGN(0x1000);
    _end = .;
    
    /DISCARD/ : {
        *(.eh_frame)
        *(.note*)
        *(.comment*)
    }
}