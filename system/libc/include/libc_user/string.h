#ifndef LIBC_USER_STRING_H
#define LIBC_USER_STRING_H

#include <stddef.h>

void* memcpy(void* dest, const void* src, size_t n);
void* memset(void* dest, int val, size_t n);
size_t strlen(const char* str);
int memcmp(const void* s1, const void* s2, size_t n);
char* strcpy(char* dest, const char* src);
int strcmp(const char* s1, const char* s2);
char* strtok(char* str, const char* delim);
char* strchr(const char* str, int c);

#endif // LIBC_USER_STRING_H
