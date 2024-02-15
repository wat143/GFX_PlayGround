#include <algorithm>
#include <iostream>
#include <cstring>
#include <WaylandDisplay.h>

/* registry callbacks */
static void registry_handle_global(void *data, struct wl_registry *registry,
                                   uint32_t name, const char *interface,
                                   uint32_t version) {
    struct display *display = static_cast<struct display*>(data);
    if (strcmp(interface, "wl_compositor") == 0) {
        display->compositor =
            (struct wl_compositor*)wl_registry_bind(registry, name, &wl_compositor_interface, 1);
    } else if (strcmp(interface, "xdg_wm_base") == 0) {
        display->xdg_wm_base =
            (struct xdg_wm_base*)wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
    }
}

static void registry_handle_global_remove(void *data, struct wl_registry *registry,
                                          uint32_t name) {
    // Handle global removal if needed
}

static const struct wl_registry_listener registry_listener {
    registry_handle_global,
    registry_handle_global_remove
};

static void
xdg_wm_base_ping(void *data, struct xdg_wm_base *shell, uint32_t serial)
{
	xdg_wm_base_pong(shell, serial);
}

static const struct xdg_wm_base_listener wm_base_listener = {
	xdg_wm_base_ping,
};

static void
handle_surface_configure(void *data, struct xdg_surface *surface,
			 uint32_t serial)
{
    struct display* display = static_cast<struct display*>(data);
	xdg_surface_ack_configure(surface, serial);
    display->configure = true;
}

static const struct xdg_surface_listener xdg_surface_listener = {
	handle_surface_configure
};

static void handle_toplevel_configure(void* data, struct xdg_toplevel* toplevel,
                                      int32_t width, int32_t height,
                                      struct wl_array* states) {
    struct display* display = static_cast<struct display*>(data);
    if (width != 0 && height != 0) {
        display->width = width;
        display->height = height;
    }
    else {
        display->width = 1920;
        display->height = 1080;
    }
}

static void
handle_toplevel_close(void *data, struct xdg_toplevel *xdg_toplevel) {}

static const struct xdg_toplevel_listener xdg_toplevel_listener = {
    handle_toplevel_configure,
    handle_toplevel_close
};

WaylandDisplay::WaylandDisplay(int type):NativeDisplay(type){
    display = new struct display;

    /* connect display */
    display->display = wl_display_connect(nullptr);
    if (!display->display) {
        std::cerr << "Failed to conenct wayland display\n";
        return;
    }

    /* get and bind necessary registries */
    display->registry = wl_display_get_registry(display->display);
    wl_registry_add_listener(display->registry, &registry_listener, display);
    wl_display_dispatch(display->display);
    wl_display_roundtrip(display->display);
    if (!display->compositor) {
        std::cerr << "Cannot bind essential globals\n";
        wl_display_disconnect(display->display);
        return;
    }

    /* create surface */
    display->surface = wl_compositor_create_surface(display->compositor);
    xdg_wm_base_add_listener(display->xdg_wm_base, &wm_base_listener, display);
    display->xdg_surface =
        xdg_wm_base_get_xdg_surface(display->xdg_wm_base, display->surface);
    xdg_surface_add_listener(display->xdg_surface, &xdg_surface_listener, display);
    if (!display->surface || !display->xdg_surface) {
        std::cerr << "Failed to create surface\n";
        return;
    }

    /* create xdg_toplevel */
    display->xdg_toplevel = xdg_surface_get_toplevel(display->xdg_surface);
    xdg_toplevel_add_listener(display->xdg_toplevel, &xdg_toplevel_listener, display);
    xdg_toplevel_set_title(display->xdg_toplevel, "Wayland display");
    xdg_toplevel_set_app_id(display->xdg_toplevel, "wayland.display");
    xdg_toplevel_set_fullscreen(display->xdg_toplevel, nullptr);
    xdg_toplevel_set_maximized(display->xdg_toplevel);

    /* wait for configure event to get initial surface state */
    display->configure = false;
    while (!display->configure) {
        wl_display_dispatch(display->display);
    }
    window_w = display->width;
    window_h = display->height;

    /* create wl_egl_window */
    display->egl_window = wl_egl_window_create(display->surface, window_w, window_h);
    if (!display->egl_window) {
    std::cerr << "Failed to create wl_egl_window\n";
        wl_display_disconnect(display->display);
        return;
    }
}

WaylandDisplay::~WaylandDisplay(){
    wl_shell_surface_destroy(display->shell_surface);
	wl_surface_destroy(display->surface);
    wl_egl_window_destroy(display->egl_window);
    wl_display_disconnect(display->display);
    delete display;
}

void* WaylandDisplay::getNativeDisplay() {
    return display->display;
}

void* WaylandDisplay::getNativeWindow() {
    return display->egl_window;
}

void* WaylandDisplay::getDisplayDev() {
    return display->display;
}

int WaylandDisplay::pageFlip() {
    /* NOP */
    return 0;
}
