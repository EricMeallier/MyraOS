#ifndef UI_H
#define UI_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_WIDGETS 64

typedef struct widget_t {
    int x, y, width, height;

    bool dirty;
    bool visible;
    bool destroy;

    void (*draw)(struct widget_t*);
    void (*on_click)(struct widget_t*, int rel_x, int rel_y);

    void* data;
} widget_t;

void ui_clear(void);
void ui_add_widget(widget_t* w);
void ui_render(void);
void ui_handle_click(int x, int y);
void ui_destroy_widget(widget_t* w);
void ui_set_dirty(widget_t* w);

#endif // UI_H