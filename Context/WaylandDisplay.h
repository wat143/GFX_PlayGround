#ifndef __WAYLAND_DISPLAY__
#define __WAYLAND_DISPLAY__

#include <Display.h>
#include <wayland-client.h>
#include <wayland-client-protocol.h>
#include <wayland-egl.h>
#include "xdg-shell-client-protocol.h"

#include "Utils.h"

struct display {
	struct wl_display* display;
	struct wl_registry* registry;
	struct wl_compositor* compositor;
    struct wl_surface* surface;
    struct wl_shell* shell;
    struct wl_shell_surface* shell_surface;
    struct wl_egl_window* egl_window;
    struct xdg_wm_base* xdg_wm_base;
    struct xdg_surface* xdg_surface;
    struct xdg_toplevel* xdg_toplevel;
    uint32_t width, height;
    bool configure;
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
