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
    } else if (strcmp(interface, "wl_shell") == 0) {
        display->shell =
            (struct wl_shell*)wl_registry_bind(registry, name, &wl_shell_interface, 1);
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

void handle_ping(void *data, struct wl_shell_surface *shell_surface, uint32_t serial) {
    wl_shell_surface_pong(shell_surface, serial);
}

const struct wl_shell_surface_listener shell_surface_listener = {
    .ping = handle_ping,
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

    /* create wl_surface and wl_shell */
    display->surface = wl_compositor_create_surface(display->compositor);
    if (!display->shell) {
        std::cerr << "Failed to create wl_shell\n";
        wl_display_disconnect(display->display);
        return;
    }

    display->shell_surface = wl_shell_get_shell_surface(display->shell, display->surface);
    if (!display->shell_surface) {
        std::cerr << "Failed to create wl_shell_surface\n";
        wl_display_disconnect(display->display);
        return;
    }

    /* Set the title of the window */
    wl_shell_surface_set_title(display->shell_surface, "Wayland Display");
    /* Add a listener for the shell surface events (e.g., ping) */
    wl_shell_surface_add_listener(display->shell_surface, &shell_surface_listener, display);
    // Map the surface, making it visible
    wl_shell_surface_set_toplevel(display->shell_surface);

    /* create wl_egl_window */
    display->egl_window = wl_egl_window_create(display->surface, 1024, 720);
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
