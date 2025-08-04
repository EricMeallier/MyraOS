#include "os_widgets/top_bar_widget.h"

#include "font/fonts/myra_font.h"
#include "gfx/gfx.h"
#include "heap/heap.h"
#include "libc_kernel/string.h"
#include "rtc/rtc.h"
#include "ui/widgets/label_widget.h"

typedef struct top_bar_data_t {
    argb_t color;
} top_bar_data_t;

static void top_bar_draw(widget_t* w) {
    font_state_t font_state = font_save_state();

    top_bar_data_t* data = (top_bar_data_t*) w->data;

    // Draw background
    gfx_fill_rect(LAYER_UI, w->x, w->y, w->width, w->height, data->color);

    // Draw time
    datetime_t dt = rtc_get_system_datetime();
    font_set_cursor((cursor_t){10, w->y + (w->height - myra_font.height) / 2});
    font_writef("%02d/%02d %02d:%02d:%02d", dt.day, dt.month, dt.hour, dt.minute, dt.second);

    // Draw "MyraOS" label
    const char* title = "MyraOS";
    int title_x = (w->width - kstrlen(title) * myra_font.width) / 2;
    int title_y = w->y + (w->height - myra_font.height) / 2;

    font_set_cursor((cursor_t){title_x, title_y});
    font_write(title);

    font_restore_state(font_state);
}

widget_t* top_bar_create(argb_t color, int x, int y, int height) {
    top_bar_data_t* data = kmalloc(sizeof(top_bar_data_t));
    data->color = color;

    widget_t* w = kmalloc(sizeof(widget_t));
    w->x = x;
    w->y = y;
    w->width = fb_info.width;
    w->height = height;
    w->data = (void*) data;
    w->visible = true;
    w->dirty = true;
    w->destroy = false;
    w->draw = top_bar_draw;
    w->on_click = NULL;

    return w;
}