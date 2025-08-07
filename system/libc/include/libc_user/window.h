#ifndef LIBC_USER_WINDOW_H
#define LIBC_USER_WINDOW_H

#include <stdint.h>

typedef uint32_t window_t;
typedef uint32_t win_handle_t;

typedef struct {
    uint32_t w;
    uint32_t h;
} win_surface_info_t;

window_t window_create(uint32_t w, uint32_t h, uint32_t flags);
int window_get_surface(window_t h, void** out_pixels, uint32_t* out_pitch, win_surface_info_t* out_info);
int window_present(window_t h);
int window_destroy(window_t h);
int window_set_title(win_handle_t h, const char* title);

#endif // LIBC_USER_WINDOW_H
