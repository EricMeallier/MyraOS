#ifndef STRING_H
#define STRING_H

#include <stddef.h>

// Strings
int kstrcmp(const char* s1, const char* s2);
size_t kstrlen(const char* str);

// Memory
void* kmemcpy(void* dest, const void* src, size_t size);
void* kmemset(void* dest, int ch, size_t count);

#endif // STRING_H
