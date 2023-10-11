#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <wayland-client.h>
#include <wayland-client-protocol.h>
#include <wayland-egl.h>
#include "xdg-shell-client-protocol.h"
#include "ringBuffGst.h"

struct egl {
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
};

struct gl {
    GLuint vshader;
    GLuint fshader;
    GLuint program;
    GLuint attrPos;
    GLuint attrCoord;
    GLuint uniTex;
    GLuint texture;
    GLuint ibo;
    GLuint vbo_pos;
    GLuint vbo_coord;
};

// Wayland display
struct display {
    struct egl* egl;
    struct gl* gl;
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
    struct ringBuff* ringBuff;
    GMainLoop *main_loop;
    uint32_t width, height;
    gint videoWidth, videoHeight;
    const gchar* format;
    bool configure;
    bool pending;
    bool eos;
};

// Shaders
const GLchar* vertex_source = 
    "attribute vec3 pos;"
    "attribute vec2 coord;"
    "varying vec2 tcoord;"
    "void main(void) {"
    " tcoord = coord;"
    " gl_Position = vec4(pos, 1.0);"
    "}";

const GLchar* fragment_source =
    "precision mediump float;"
    "uniform sampler2D tex;"
    "varying vec2 tcoord;"
    "void main(void) {"
    "  vec4 color = texture2D(tex, tcoord);"
    "  gl_FragColor = color;"
    "}";
