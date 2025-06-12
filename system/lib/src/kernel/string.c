#include "kernel/string.h"

void* kmemcpy(void* dest, const void* src, size_t size) {
    char *csrc = (char*)src; 
    char *cdest = (char*)dest; 

    for (size_t i = 0; i < size; i++) {
        cdest[i] = csrc[i];
    } 

    return dest;
}

int kstrcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }

    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}
