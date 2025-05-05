ENTRY(_start)
OUTPUT_FORMAT("binary")

SECTIONS {
    . = 0x10000;
    
    .entry : {
        *(.entry)
    }
    
    .text : {
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
    
    /DISCARD/ : {
        *(.eh_frame)
        *(.note*)
        *(.comment*)
    }
}