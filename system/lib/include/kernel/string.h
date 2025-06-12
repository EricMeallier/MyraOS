#ifndef STRING_H
#define STRING_H

#include <stddef.h>

void* kmemcpy(void* dest, const void* src, size_t size);
int kstrcmp(const char* s1, const char* s2);

#endif // STRING_H