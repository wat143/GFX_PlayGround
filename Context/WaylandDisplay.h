#ifndef __WAYLAND_DISPLAY__
#define __WAYLAND_DISPLAY__

#include <Display.h>
#include <wayland-client.h>
#include <wayland-client-protocol.h>
#include <wayland-egl.h>

#include "Utils.h"

struct display {
	struct wl_display* display;
	struct wl_registry* registry;
	struct wl_compositor* compositor;
    struct wl_surface* surface;
    struct wl_shell* shell;
    struct wl_shell_surface* shell_surface;
    struct wl_egl_window* egl_window;
};

class WaylandDisplay : public NativeDisplay {
private:
    struct display* display;
    void createSurface(struct window* window);
public:
    WaylandDisplay(int type);
    ~WaylandDisplay();
    void* getNativeDisplay();
    void* getNativeWindow();
    void* getDisplayDev();
    int pageFlip();
};

#endif
