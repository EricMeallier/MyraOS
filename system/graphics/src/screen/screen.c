#include "screen/screen.h"

#include "font/fonts/myra_font.h"
#include "os_widgets/top_bar_widget.h"
#include "ui/widgets/image_widget.h"
#include "ui/widgets/background_widget.h"
#include "ui/widgets/label_widget.h"
#include "ui/ui.h"

screen_t screen_desktop = {
    .init = screen_desktop_init,
    .handle_input = NULL,
};

void screen_desktop_init(void) {
    ui_add_widget(widget_os_top_bar_create(0, 0, 40, 0xFF111111));

    image_t* boot_image = image_parse("/myra/boot/boot_image.bmp");
    ui_add_widget(widget_image_create((fb_info.width / 2) - (boot_image->width / 2), (fb_info.height / 2) - (boot_image->height / 2), boot_image));
    ui_add_widget(widget_background_create(0xFF222222));
    ui_add_widget(widget_label_create(50, 50, "Welcome to MyraOS! hi!", &myra_font, 0xFFFFFFFF));
}