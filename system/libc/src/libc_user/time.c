#include <stdint.h>

#define SYS_TIME              13
#define SYS_NANOSLEEP         162

static inline int syscall(int num, int arg1, int arg2, int arg3) {
    int ret;
    asm volatile (
        "int $0x80"
        : "=a"(ret)
        : "a"(num), "b"(arg1), "c"(arg2), "d"(arg3)
    );
    return ret;
}

int sleep(uint32_t ms) {
    syscall(SYS_NANOSLEEP, ms, 0, 0);
    return 0;
}

uint32_t time(void) {
    return syscall(SYS_TIME, 0, 0, 0);
}