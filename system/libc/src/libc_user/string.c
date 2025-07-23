#include <string.h>

void* memcpy(void* dest, const void* src, size_t n) {
    unsigned char* d = dest;
    const unsigned char* s = src;

    while (n--) {
        *d++ = *s++;
    }

    return dest;
}

void* memset(void* dest, int val, size_t n) {
    unsigned char* d = dest;

    while (n--) {
        *d++ = (unsigned char) val;
    }

    return dest;
}

size_t strlen(const char* str) {
    size_t len = 0;

    while (str[len]) {
        len++;
    }

    return len;
}

int memcmp(const void* s1, const void* s2, size_t n) {
    const unsigned char *a = s1, *b = s2;

    for (size_t i = 0; i < n; i++) {
        if (a[i] != b[i]) {
            return a[i] - b[i];
        }
    }

    return 0;
}

char* strcpy(char* dest, const char* src) {
    char* d = dest;

    while ((*d++ = *src++));

    return dest;
}

int strcmp(const char* s1, const char* s2) {
    while (*s1 && *s1 == *s2) {
        s1++, s2++;
    }

    return *(unsigned char*)s1 - *(unsigned char*)s2;
}
