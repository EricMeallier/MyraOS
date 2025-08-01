#include "frame/frame.h"

#include <stdbool.h>

#include "gfx/gfx.h"
#include "mouse/mouse.h"
#include "libc_kernel/string.h"
#include "ui/ui.h"

static screen_t* current_screen;

void frame_render(void) {
    if (current_screen) {
        ui_render();
    }

    gfx_flush_dirty();
}

void frame_set_screen(screen_t* new_screen) {
    ui_clear();

    current_screen = new_screen;
    if (current_screen->init) {
        current_screen->init();
    }
}