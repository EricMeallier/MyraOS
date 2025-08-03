#ifndef LABEL_WIDGET_H
#define LABEL_WIDGET_H

#include "fb/fb.h"
#include "font/font.h"
#include "ui/ui.h"

widget_t* label_create(int x, int y, const char* text, font_t* font, argb_t color);

#endif // LABEL_WIDGET_H