ENTRY(_start)
OUTPUT_FORMAT("binary")

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