#ifndef LIBC_USER_TIME_H
#define LIBC_USER_TIME_H

#include <stdint.h>

int sleep(uint32_t ms);
uint32_t time(void);

#endif // LIBC_USER_TIME_H