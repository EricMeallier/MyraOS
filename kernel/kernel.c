void kernel_main() {
    const char *msg = "Welcome to MyraOS Kernel!";
    char *video_memory = (char *)0xB8000;

    for (int i = 0; msg[i] != 0; i++) {
        video_memory[i * 2] = msg[i];  
        video_memory[i * 2 + 1] = 0x0F; 
    }

    while (1) {
        __asm__ volatile("hlt");
    }
}