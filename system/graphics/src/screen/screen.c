#include "screen/screen.h"

#include "font/fonts/myra_font.h"
#include "os_widgets/top_bar_widget.h"
#include "ui/widgets/label_widget.h"
#include "ui/ui.h"

screen_t screen_desktop = {
    .init = screen_desktop_init,
    .handle_input = NULL,
};

void screen_desktop_init(void) {
    ui_add_widget(top_bar_create(0xFF010101, 0, 0, 40));
    ui_add_widget(label_create(50, 50, "Welcome to MyraOS! hi!", &myra_font, 0xFFFFFFFF));
}