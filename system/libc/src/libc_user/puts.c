#include <unistd.h>
#include <string.h>
#include <stdio.h>

int puts(const char* str) {
    int len = strlen(str);

    write(1, str, len);
    write(1, "\n", 1);

    return len + 1;
}
