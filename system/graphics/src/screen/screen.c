#include "screen/screen.h"

#include "font/fonts/myra_font.h"
#include "ui/widgets/label_widget.h"
#include "ui/ui.h"

screen_t screen_desktop = {
    .init = screen_desktop_init,
    .handle_input = NULL,
};

void screen_desktop_init(void) {
    ui_add_widget(label_create(50, 50, "Welcome to MyraOS! hi!", &myra_font, 0xFFFFFFFF));
}