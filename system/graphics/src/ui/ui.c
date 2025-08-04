#include "ui/ui.h"

#include <stddef.h>

#include "heap/heap.h"

static widget_t* ui_widgets[MAX_WIDGETS];
static size_t ui_widget_count = 0;

void ui_clear(void) {
    for (size_t i = 0; i < ui_widget_count; i++) {
        widget_t* w = ui_widgets[i];

        if (w->data) {
            kfree(w->data);
        }

        kfree(w);
        ui_widgets[i] = NULL;
    }

    ui_widget_count = 0;
}

void ui_add_widget(widget_t* w) {
    if (ui_widget_count == MAX_WIDGETS) {
        return;
    }

    ui_widgets[ui_widget_count++] = w;
}

void ui_render(void) {
    for (size_t i = 0; i < ui_widget_count; i++) {
        widget_t* w = ui_widgets[i];
        if (w->destroy) {
            for (size_t j = i; j < ui_widget_count - 1; j++) {
                ui_widgets[j] = ui_widgets[j + 1];
            }
            ui_widget_count--;
        } else if (w->visible && w->draw && w->dirty) {
            w->draw(w);
            w->dirty = false;
        }
    }
}

void ui_handle_click(uint32_t x, uint32_t y) {
    for (int i = ui_widget_count - 1; i >= 0; i--) {
        widget_t* w = ui_widgets[i];
        if (!w->visible || !w->on_click) {
            continue;
        }

        if (x >= w->x && x < w->x + w->width && y >= w->y && y < w->y + w->height) {
            uint32_t rel_x = x - w->x;
            uint32_t rel_y = y - w->y;

            w->on_click(w, rel_x, rel_y);
            
            return; 
        }
    }
}

void ui_destroy_widget(widget_t* w) {
    w->destroy = true;
}

void ui_set_dirty(widget_t* w) {
    w->dirty = true;
}