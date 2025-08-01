#include "frame/frame.h"

#include <stdbool.h>

#include "gfx/gfx.h"
#include "mouse/mouse.h"
#include "libc_kernel/string.h"

static screen_t current_screen;

void frame_render(void) {
    if (current_screen.draw) {
        current_screen.draw();
    }

    gfx_flush_dirty();
}

void frame_set_screen(screen_t* new_screen) {
    current_screen.draw = new_screen->draw;
    current_screen.update = new_screen->update;
    current_screen.handle_input = new_screen->handle_input;
}