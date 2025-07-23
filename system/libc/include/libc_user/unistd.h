#ifndef LIBC_USER_UNISTD_H
#define LIBC_USER_UNISTD_H

#include <stddef.h>
#include <sys/types.h>

int write(int fd, const void* buf, size_t count);
void _exit(int status);
pid_t fork(void);
int execve(const char* path, char* const argv[], char* const envp[]);
int read(int fd, void* buf, size_t count);
int open(const char* path, int flags);
int close(int fd);

#endif // LIBC_USER_UNISTD_H