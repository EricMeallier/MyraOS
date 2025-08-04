#ifndef WIDGET_IMAGE_H
#define WIDGET_IMAGE_H

#include "ui/ui.h"
#include "image/image_loader.h"

widget_t* widget_image_create(uint32_t x, uint32_t y, const image_t* image);

#endif // WIDGET_IMAGE_H
