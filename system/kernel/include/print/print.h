//
// Created by dvir on 5/19/25.
//

#ifndef PRINT_H
#define PRINT_H

#include <stdarg.h>
#include <stddef.h>

typedef enum color_t { 
    COLOR_WHITE,
    COLOR_YELLOW,
    COLOR_RED,
    COLOR_GREEN,
    COLOR_BLUE,
} color_t;

// print
void kprint(const char *str);

void kprintln(const char *str);

void kprintf(const char *str, ...);

// clear
void kclear_screen(void);

void kclear(size_t n);

// set color
void kset_color(const color_t color);

#endif  // PRINT_H
