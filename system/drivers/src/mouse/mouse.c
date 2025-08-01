#include "mouse/mouse.h"

#include <stdbool.h>

#include "fb/fb.h"
#include "gfx/gfx.h"
#include "io/port_io.h"
#include "interrupt/irq/irq.h"
#include "libc_kernel/string.h"

#include "font/font.h"

#define CURSOR_SIZE 3
#define CURSOR_SAFE_MARGIN 1
#define CURSOR_AREA_SIZE (CURSOR_SIZE + CURSOR_SAFE_MARGIN)

static int8_t mouse_cycle = 0;
static int8_t mouse_bytes[3];

static int32_t prev_mouse_x = -1, prev_mouse_y = -1;
static int32_t mouse_x = 0, mouse_y = 0;

static argb_t prev_saved_pixels[CURSOR_AREA_SIZE * 2][CURSOR_AREA_SIZE * 2];

static bool cursor_saved = false;
static bool cursor_enabled = false;

static void draw_mouse(void);
static void save_cursor_bg(int x, int y, argb_t buffer[CURSOR_AREA_SIZE * 2][CURSOR_AREA_SIZE * 2]);
static void restore_cursor_bg(int x, int y, argb_t buffer[CURSOR_AREA_SIZE * 2][CURSOR_AREA_SIZE * 2]);
static void mouse_handler(registers_t* _);

void mouse_init(void) {
    outb(0x64, 0xA8);
    outb(0x64, 0x20);
    
    uint8_t status = inb(0x60);
    outb(0x64, 0x60);
    outb(0x60, status | 2);

    mouse_write(0xF6); mouse_read();
    mouse_write(0xF4); mouse_read();

    irq_install_handler(MOUSE_IRQ, mouse_handler);
}

void mouse_write(uint8_t val) {
    while (inb(0x64) & 2);
    outb(0x64, 0xD4);
    while (inb(0x64) & 2);
    outb(0x60, val);
}

uint8_t mouse_read(void) {
    while (!(inb(0x64) & 1));
    return inb(0x60);
}

void mouse_set(bool is_enabled) {
    cursor_enabled = is_enabled;
}

static void mouse_handler(registers_t* _) {
    (void) _;

    uint8_t data = inb(0x60);
    mouse_bytes[mouse_cycle++] = data;

    if (mouse_cycle == 3) {
        int8_t dx = mouse_bytes[1];
        int8_t dy = mouse_bytes[2];
        mouse_x += dx;
        mouse_y -= dy;

        if (mouse_x < 0) {
            mouse_x = 0;
        } else if ((uint32_t) mouse_x > fb_info.width) {
            mouse_x = fb_info.width;
        }

        if (mouse_y < 0) {
            mouse_y = 0;
        } else if ((uint32_t) mouse_y > fb_info.height) {
            mouse_y = fb_info.height;
        } 

        mouse_cycle = 0;

        draw_mouse();
    }
}

static void draw_mouse(void) { 
    if (!cursor_enabled) {
        return;
    }

    if (cursor_saved) {
        restore_cursor_bg(prev_mouse_x, prev_mouse_y, prev_saved_pixels);
    }
        
    save_cursor_bg(mouse_x, mouse_y, prev_saved_pixels);

    gfx_fill_circle(mouse_x, mouse_y, CURSOR_SIZE, 0xFFFFFFFF);
    gfx_flush_dirty();

    prev_mouse_x = mouse_x;
    prev_mouse_y = mouse_y;

    cursor_saved = true;
}

static void save_cursor_bg(int x, int y, argb_t buffer[CURSOR_AREA_SIZE * 2][CURSOR_AREA_SIZE * 2]) {
    for (int dy = -CURSOR_AREA_SIZE; dy < CURSOR_AREA_SIZE; dy++) {
        for (int dx = -CURSOR_AREA_SIZE; dx < CURSOR_AREA_SIZE; dx++) {
            int px = x + dx;
            int py = y + dy;
            if (px >= 0 && py >= 0 && (uint32_t)px < fb_info.width && (uint32_t)py < fb_info.height) {
                buffer[dy + CURSOR_AREA_SIZE][dx + CURSOR_AREA_SIZE] = gfx_get_pixel(px, py);
            }
        }
    }
}

static void restore_cursor_bg(int x, int y, argb_t buffer[CURSOR_AREA_SIZE * 2][CURSOR_AREA_SIZE * 2]) {
    for (int dy = -CURSOR_AREA_SIZE; dy < CURSOR_AREA_SIZE; dy++) {
        for (int dx = -CURSOR_AREA_SIZE; dx < CURSOR_AREA_SIZE; dx++) {
            int px = x + dx;
            int py = y + dy;
            if (px >= 0 && py >= 0 && (uint32_t)px < fb_info.width && (uint32_t)py < fb_info.height) {
                gfx_draw_pixel(px, py, buffer[dy + CURSOR_AREA_SIZE][dx + CURSOR_AREA_SIZE]);
            }
        }
    }
}