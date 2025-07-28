#include <stdbool.h>

#include "ext2/ext2.h"
#include "gdt/gdt.h"
#include "heap/heap.h"
#include "interrupt/idt/idt.h"
#include "interrupt/irq/irq.h"
#include "interrupt/isr/isr.h"
#include "interrupt/syscall/syscall.h"
#include "keyboard/keyboard.h"
#include "block_device/pata.h"
#include "pit/pit.h"
#include "pmm/pmm.h"
#include "print/print.h"
#include "rtc/rtc.h"
#include "schedule/schedule.h"
#include "tty/tty.h"
#include "vmm/vmm.h"

extern uint32_t multiboot_info_addr;
extern void parse_multiboot_info(uint32_t addr);

void kernel_main(void) {
    pmm_init();
    vmm_init();
    
    heap_init(HEAP_START_ADDR, HEAP_SIZE);

    gdt_init();

    idt_init();
    isr_init();
    irq_init();
    syscall_init();

    pit_init();
    rtc_init();

    keyboard_driver_install();
    tty_init();

    pata_init();

    block_device_t* block_device = block_get_device("hd0");
    if (!block_device) {
        klog_error("hd0 not found");
    }

    root_fs = (ext2_fs_t*) kmalloc(sizeof(ext2_fs_t));
    if (!ext2_mount(root_fs, block_device)) {
        klog_error("FS mount failed");
    }

    size_t shell_elf_size = 0;
    bool succeeded = true;
    void* shell_elf_data = (void*) ext2_read_file(root_fs, "/bin/shell.elf", &shell_elf_size, &succeeded);
    if (!succeeded) {
        klog_error("Loading shell ELF failed");
    }

    elf_load_info_t shell_load_info;
    if (!elf_parse(shell_elf_data, &shell_load_info)) {
        klog_error("ELF parsing failed");
    }

    exec_info_t shell_exec_info;
    exec_parse_info_elf(&shell_load_info, &shell_exec_info);

    kprint("Welcome to \x1b[32mMyraOS\x1b[0m\n");

    datetime_t dt = rtc_get_system_datetime();
    kprintf("Current Time: %02d:%02d:%02d, Date: %02d/%02d/%d (Weekday: %d)\n\n",
            dt.hour, dt.minute, dt.second,
            dt.day, dt.month, dt.year,
            dt.weekday
        );

    parse_multiboot_info(multiboot_info_addr);

    schedule_init(&shell_exec_info);

    while (true) {
        __asm__ volatile("hlt");
    }
}
