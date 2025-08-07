#include "fb/fb.h"
#include "window/window.h"
#include "libc_kernel/string.h"
#include "heap/heap.h"
#include "os_widgets/window_widget.h"
#include "schedule/schedule.h"
#include "ui/ui.h"
#include "vmm/vmm.h"

#define WIN_SURF_UBASE  0x30000000u

static inline uint32_t user_base_for_handle(win_handle_t h) {
    return WIN_SURF_UBASE + (h * 0x01000000u);
}

uint32_t sys_window_create(uint32_t w, uint32_t h, uint32_t flags, uint32_t _) {
    (void)_;

    if (schedule_current_proc == NULL) {
        return (uint32_t)-1;
    }

    win_handle_t handle = window_create(schedule_current_proc->pid, w, h, flags);

    return (uint32_t)handle;
}

uint32_t sys_window_get_surface(uint32_t handle, uint32_t out_pixels_ptr, uint32_t out_pitch_ptr, uint32_t out_info_ptr) {
    window_t* win = window_from_handle((win_handle_t)handle);
    if (!win || win->owner_pid != schedule_current_proc->pid) {
        return (uint32_t)-1;
    }

    if (win->surface_uptr == NULL) {
        uint32_t ubase = user_base_for_handle(win->handle);

        for (uint32_t i = 0; i < win->npages; i++) {
            vmm_map_page(ubase + i * PAGE_SIZE, win->frames[i], PAGE_PRESENT | PAGE_WRITE | PAGE_USER);
        }

        win->surface_uptr = (void*) ubase;
    }

    if (!out_pixels_ptr || !out_pitch_ptr || !out_info_ptr) {
        return (uint32_t)-1;
    }

    *(void**)     out_pixels_ptr = win->surface_uptr;
    *(uint32_t*)  out_pitch_ptr  = win->pitch;

    win_surface_info_t* info = (win_surface_info_t*) out_info_ptr;
    info->w = win->logical_w;
    info->h = win->logical_h;

    return 0;
}

uint32_t sys_window_destroy(uint32_t handle, uint32_t _, uint32_t __, uint32_t ___) {
    (void)_; (void)__; (void)___;

    window_t* win = window_from_handle((win_handle_t) handle);
    if (!win || win->owner_pid != schedule_current_proc->pid) {
        return (uint32_t)-1;
    }

    window_destroy((win_handle_t) handle);

    return 0;
}

uint32_t sys_window_present(uint32_t handle, uint32_t _, uint32_t __, uint32_t ___) {
    (void)_; (void)__; (void)___;

    window_t* win = window_from_handle((win_handle_t)handle);
    if (!win || win->owner_pid != schedule_current_proc->pid) {
        return (uint32_t)-1;
    }

    if (!win->has_widget) {
        const char* title = win->title ? win->title : "App";

        uint32_t scaled_w = fb_info.width;
        uint32_t scaled_h = fb_info.height - 158;
        widget_t* w = widget_os_window_create((char*) title, handle, 0, 40, scaled_w, scaled_h);

        ui_add_widget(w);
        win->has_widget = true;
    }

    window_present((win_handle_t)handle);
    return 0;
}

uint32_t sys_window_set_title(uint32_t handle, uint32_t user_title_ptr, uint32_t _, uint32_t __) {
    (void)_; (void)__;

    window_t* win = window_from_handle((win_handle_t)handle);
    if (!win || win->owner_pid != schedule_current_proc->pid) {
        return (uint32_t)-1;
    }

    const char* title = (const char*) user_title_ptr;
    if (!title) return (uint32_t)-1;

    if (win->title) {
        kfree(win->title);
    }

    char* ktitle = kmalloc(64);
    size_t i = 0;
    while (i < 63 && title[i]) {
        ktitle[i] = title[i];
        i++;
    }
    ktitle[i] = '\0';

    win->title = ktitle;

    return 0;
}
