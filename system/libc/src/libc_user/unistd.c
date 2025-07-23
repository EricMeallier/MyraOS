#include <unistd.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#define SYS_EXIT     1
#define SYS_FORK     2
#define SYS_READ     3
#define SYS_WRITE    4
#define SYS_OPEN     5
#define SYS_CLOSE    6
#define SYS_EXECVE   11

static inline int syscall(int num, int arg1, int arg2, int arg3) {
    int ret;
    asm volatile (
        "int $0x80"
        : "=a"(ret)
        : "a"(num), "b"(arg1), "c"(arg2), "d"(arg3)
    );
    return ret;
}

int write(int fd, const void* buf, size_t count) {
    return syscall(SYS_WRITE, fd, (int)buf, count);
}

void _exit(int status) {
    syscall(SYS_EXIT, status, 0, 0);
    for (;;);
}

int read(int fd, void* buf, size_t count) {
    return syscall(SYS_READ, fd, (int)buf, count);
}

int open(const char* path, int flags) {
    return syscall(SYS_OPEN, (int)path, flags, 0);
}

int close(int fd) {
    return syscall(SYS_CLOSE, fd, 0, 0);
}

pid_t fork(void) {
    return syscall(SYS_FORK, 0, 0, 0);
}

int execve(const char* path, char* const argv[], char* const envp[]) {
    return syscall(SYS_EXECVE, (int)path, (int)argv, (int)envp);
}