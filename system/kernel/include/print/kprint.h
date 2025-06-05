//
// Created by dvir on 5/19/25.
//

#ifndef KPRINT_H
#define KPRINT_H

#include <stdarg.h>

typedef enum color_t { COLOR_WHITE, COLOR_GREEN, COLOR_RED } color_t;

// print
void kprint(const char *str);

void kprintln(const char *str);

void kprintf(const char *str, ...);

// clear screen
void kclear_screen(void);

// set color
void kset_color(const color_t color);

#endif  // KPRINT_H
