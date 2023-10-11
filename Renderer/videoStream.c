#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <gst/gst.h>
#include <gst/video/video.h>
#include <gst/app/gstappsink.h>
#include <glib.h>
#include "videoStream.h"

#define FILE_PATH "../Resources/BigBuckBunny_320x180.mp4"

/***** Wayland inititalization *****/
/* registry callbacks */
static void registry_handle_global(void *data, struct wl_registry *registry,
                                   uint32_t name, const char *interface,
                                   uint32_t version) {
    struct display *display = (struct display*)data;
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

static const struct wl_registry_listener registry_listener = {
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
    struct display* display = (struct display*)data;
    xdg_surface_ack_configure(surface, serial);
    display->configure = true;
}

static const struct xdg_surface_listener xdg_surface_listener = {
    handle_surface_configure
};

static void handle_toplevel_configure(void* data, struct xdg_toplevel* toplevel,
                                      int32_t width, int32_t height,
                                      struct wl_array* states) {
    struct display* display = (struct display*)data;
    display->width = width;
    display->height = height;
}

static void
handle_toplevel_close(void *data, struct xdg_toplevel *xdg_toplevel) {}

static const struct xdg_toplevel_listener xdg_toplevel_listener = {
    handle_toplevel_configure,
    handle_toplevel_close
};

struct display* initWayland() {
    struct display* display = (struct display*)malloc(sizeof(struct display));
    /* connect display */
    display->display = wl_display_connect(NULL);
    if (!display->display) {
        fprintf(stderr, "Failed to create wl_display\n");
        goto Fail;
    }

    /* get and bind necessary registries */
    display->registry = wl_display_get_registry(display->display);
    wl_registry_add_listener(display->registry, &registry_listener, display);
    wl_display_dispatch(display->display);
    wl_display_roundtrip(display->display);
    if (!display->compositor) {
        fprintf(stderr, "Cannot bind essential globals\n");
        wl_display_disconnect(display->display);
	goto Fail;
    }

    /* create surface */
    display->surface = wl_compositor_create_surface(display->compositor);
    xdg_wm_base_add_listener(display->xdg_wm_base, &wm_base_listener, display);
    display->xdg_surface =
        xdg_wm_base_get_xdg_surface(display->xdg_wm_base, display->surface);
    xdg_surface_add_listener(display->xdg_surface, &xdg_surface_listener, display);
    if (!display->surface || !display->xdg_surface) {
        fprintf(stderr, "Failed to create surface\n");
        goto Fail;
    }

    /* create xdg_toplevel */
    display->xdg_toplevel = xdg_surface_get_toplevel(display->xdg_surface);
    xdg_toplevel_add_listener(display->xdg_toplevel, &xdg_toplevel_listener, display);
    xdg_toplevel_set_title(display->xdg_toplevel, "Wayland display");
    xdg_toplevel_set_app_id(display->xdg_toplevel, "wayland.display");
    xdg_toplevel_set_fullscreen(display->xdg_toplevel, NULL);
    xdg_toplevel_set_maximized(display->xdg_toplevel);

    /* wait for configure event to get initial surface state */
    display->configure = false;
    while (!display->configure) {
        wl_display_dispatch(display->display);
    }
    /* create wl_egl_window */
    printf("(width, height) = (%d, %d)\n", display->width, display->height);
    display->egl_window = wl_egl_window_create(display->surface, display->width, display->height);
    if (!display->egl_window) {
        fprintf(stderr, "Failed to create wl_egl_window\n");
        wl_display_disconnect(display->display);
	goto Fail;
    }
    return display;
 Fail:
    free(display);
    return NULL;
}

void deinitWayland(struct display* display) {
    wl_shell_surface_destroy(display->shell_surface);
    wl_surface_destroy(display->surface);
    wl_egl_window_destroy(display->egl_window);
    wl_display_disconnect(display->display);
    free(display);
}
/***********************************/

/***** Initialize EGL *****/
struct egl* initEGL(struct display* display) {
    struct egl* egl = (struct egl*)malloc(sizeof(struct egl));
    EGLint num_config = 0;
    int ret = 0;
    const EGLint attribute_list[] =
        {
            EGL_RED_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE, 8,
            EGL_ALPHA_SIZE, 8,
            EGL_DEPTH_SIZE, 24,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_NONE
        };

    const EGLint context_attributes[] = 
        {
            EGL_CONTEXT_CLIENT_VERSION, 2,
            EGL_NONE
        };

    // Get eglDisplay
    egl->display = eglGetDisplay((EGLNativeDisplayType)display->display);

    // Initialize the EGL display connection
    ret = eglInitialize(egl->display, NULL, NULL);

    // Get an appropriate EGL frame buffer configuration
    EGLConfig config;
    ret = eglChooseConfig(egl->display, attribute_list, &config, 1, &num_config);

    // Bind GLESv2
    eglBindAPI(EGL_OPENGL_ES_API);
   
    // Create an EGL rendering context
    egl->context =
        eglCreateContext(egl->display, config, EGL_NO_CONTEXT, context_attributes);

    // Create an EGL window surface
    egl->surface = eglCreateWindowSurface(egl->display,
                                          config,
                                          (EGLNativeWindowType)display->egl_window,
                                          NULL);
    eglMakeCurrent(egl->display, egl->surface, egl->surface, egl->context);
    return egl;
}
/***********************************/

/***** Initialize OpenGL *****/
static void showShaderInfo(GLuint shader) {
    GLsizei bufSize = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &bufSize);
    if (bufSize) {
        GLchar *log = (GLchar*)malloc(sizeof(GLchar) * bufSize);
        GLsizei len;
        glGetShaderInfoLog(shader, bufSize, &len, log);
        fprintf(stderr, "%s\n", log);
        free(log);
    }
}

static void initShader(struct gl *gl) {
    gl->vshader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(gl->vshader, 1, &vertex_source, 0);
    glCompileShader(gl->vshader);
    showShaderInfo(gl->vshader);

    gl->fshader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(gl->fshader, 1, &fragment_source, 0);
    glCompileShader(gl->fshader);
    showShaderInfo(gl->fshader);

    gl->program = glCreateProgram();
    glAttachShader(gl->program, gl->vshader);
    glAttachShader(gl->program, gl->fshader);
    glLinkProgram(gl->program);
}

struct gl* initGL(struct display* display) {
    GLint offset = 10;
    struct gl* gl = (struct gl*)malloc(sizeof(struct gl));
    const GLfloat vertexPos[] = {
        -1.0, 1.0, 0.0,
        1.0, 1.0, 0.0,
        1.0, -1.0, 0.0,
        -1.0, -1.0, 0.0
    };
    const GLfloat vertexCoord[] = {
        0.0, 0.0,
        1.0, 0.0,
        1.0, 1.0,
        0.0, 1.0
    };
    const GLushort index[] = {
        0, 1, 2,
        0, 2, 3
    };
    GLuint arrayBuf[2], elemBuf;

    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(1.0, 1.0, 1.0, 1.0);
    // init shader
    initShader(gl);
    glUseProgram(gl->program);
    assert(glGetError() == 0);
    // Get attribute and uniform location
    gl->attrPos = glGetAttribLocation(gl->program, "pos");
    gl->attrCoord = glGetAttribLocation(gl->program, "coord");
    glEnableVertexAttribArray(gl->attrPos);
    glEnableVertexAttribArray(gl->attrCoord);
    assert(glGetError() == 0);
    gl->uniTex = glGetUniformLocation(gl->program, "tex");
    assert(glGetError() == 0);
    // Set vertex pos
    glGenBuffers(2, arrayBuf);
    glBindBuffer(GL_ARRAY_BUFFER, arrayBuf[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 12, vertexPos, GL_STATIC_DRAW);
    glVertexAttribPointer(gl->attrPos, 3, GL_FLOAT, GL_TRUE, 0, 0);
    gl->vbo_pos = arrayBuf[0];
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    assert(glGetError() == 0);
    // Set texture coordinate
    glBindBuffer(GL_ARRAY_BUFFER, arrayBuf[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 8, vertexCoord, GL_STATIC_DRAW);
    glVertexAttribPointer(gl->attrCoord, 2, GL_FLOAT, GL_TRUE, 0, 0);
    gl->vbo_coord = arrayBuf[1];
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    assert(glGetError() == 0);
    // Set index buff
    glGenBuffers(1, &elemBuf);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elemBuf);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * 6, index, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    gl->ibo = elemBuf;
    // create texture and configure
    glGenTextures(1, &gl->texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glBindTexture(GL_TEXTURE_2D, gl->texture);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLfloat)GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (GLfloat)GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
    assert(glGetError() == 0);

    glViewport(offset, offset, display->width-(2*offset), display->height-2*(offset));

    return gl;
}

/***** Gstreamer related functions *****/
static GstFlowReturn on_new_sample(GstAppSink* appsink, gpointer user_data) {
    GstSample *sample = gst_app_sink_pull_sample(appsink);
    struct display *display = (struct display*)user_data;
    struct ringBuff *ringBuff = display->ringBuff;;
    static int count = 0;
    if (sample) {
        GstBuffer* buffer = gst_sample_get_buffer(sample);
        /* initialize video info */
        if (display->videoWidth == 0 || display->videoHeight == 0) {
            GstCaps *caps = gst_sample_get_caps(sample);
            if (caps) {
                GstStructure *str = gst_caps_get_structure(caps, 0);
                gst_structure_get_int(str, "width", &display->videoWidth);
                gst_structure_get_int(str, "height", &display->videoHeight);
                display->format = gst_structure_get_string(str, "format");
                gst_caps_unref(caps);
            }
            else
                fprintf(stderr, "Failed to get GstCaps\n");
            fprintf(stderr, "Initialize video info width:%d height:%d format :%s\n",
                    display->videoWidth, display->videoHeight, display->format);
        }
        /* Increase refcount of the GstBuffer */
        gst_buffer_ref(buffer);
        if(!ringBuff->push(ringBuff, buffer))
            gst_buffer_unref(buffer);
        gst_sample_unref(sample);
        display->pending = TRUE;
    }
    return GST_FLOW_OK;
}

static GstFlowReturn on_eos(GstAppSink* appsink, gpointer user_data) {
    struct display* display = (struct display*)user_data;
    fprintf(stderr, "EOS signal\n");
    display->eos = TRUE;
    g_main_loop_quit(display->main_loop);
    return GST_FLOW_OK;
}

static void on_pad_added(GstElement* element, GstPad *pad, gpointer data) {
    GstPad* sink_pad;
    GstElement* parse = (GstElement*)data;
    sink_pad = gst_element_get_static_pad(parse, "sink");
    gst_pad_link(pad, sink_pad);
    gst_object_unref(sink_pad);
}

GstElement* setupPipeline(struct display* display) {
    struct ringBuff* ringBuff = display->ringBuff;
    GstElement* pipeline = gst_pipeline_new("mypipeline");
    GstElement* src = gst_element_factory_make("filesrc", "source");
    GstElement* demux = gst_element_factory_make("qtdemux", "demuxer");
    GstElement* parse = gst_element_factory_make("h264parse", "parser");
    GstElement* decoder = gst_element_factory_make("v4l2h264dec", "decoder");
    GstElement* capsfilter = gst_element_factory_make("capsfilter", "filter");
    GstElement* converter = gst_element_factory_make("videoconvert", "converter");
    GstElement* sink = gst_element_factory_make("appsink", "sink");
    GstCaps* caps =
        gst_caps_new_simple("video/x-raw",
                            "format", G_TYPE_STRING, "RGB",
                            NULL);

    display->videoWidth = 0;
    display->videoHeight = 0;
    display->format = NULL;

    if (!pipeline || !src || !demux || !parse || !decoder || !sink) {
        g_printerr("Failed to find elements\n");
        return NULL;
    }
    // Set filesrc location
    g_object_set(G_OBJECT(src), "location", FILE_PATH, NULL);
    // Set caps
    g_object_set(G_OBJECT(capsfilter), "caps", caps, NULL);
    gst_caps_unref(caps);
    // Configure emit signal for appsink
    g_object_set(G_OBJECT(sink), "emit-signals", TRUE, NULL);
    // Connect new-sample event
    g_signal_connect_data(sink, "new-sample", G_CALLBACK(on_new_sample), display, NULL, 0);
    g_signal_connect_data(sink, "eos", G_CALLBACK(on_eos), display, NULL, 0);
    // Add elements to pipeline
    gst_bin_add_many(GST_BIN(pipeline), src, demux, parse, decoder, converter, capsfilter, sink, NULL);
    // Link elements together
    if (!gst_element_link(src, demux)) {
        g_printerr("src and demux could not be linked.\n");
        gst_object_unref(pipeline);
        return NULL;
    }
    if (!gst_element_link_many(parse, decoder, converter, capsfilter, sink, NULL)) {
        g_printerr("parse, decoder, sink could not be linked.\n");
        gst_object_unref(pipeline);
        return NULL;
    }
    g_signal_connect(demux, "pad-added", G_CALLBACK(on_pad_added), parse);

    return pipeline;
}

void draw(struct display* display) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    GstBuffer* buffer = NULL;
    GstMapInfo map;
    GstMemory* memory = NULL;
    display->pending = FALSE;
    buffer = display->ringBuff->pop(display->ringBuff);
    if (buffer == NULL) {
        fprintf(stderr, "[%s]: buffer is NULL\n", __func__);
        return;
    }
    memory = gst_buffer_get_memory(buffer, 0);
    gst_memory_map(memory, &map, GST_MAP_READ);

    glBindTexture(GL_TEXTURE_2D, display->gl->texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                 display->videoWidth, display->videoHeight,
                 0, GL_RGB, GL_UNSIGNED_BYTE, map.data);

    glUniform1i(display->gl->uniTex, 0);
    gst_memory_unmap(memory, &map);
    glBindBuffer(GL_ARRAY_BUFFER, display->gl->vbo_pos);
    glBindBuffer(GL_ARRAY_BUFFER, display->gl->vbo_coord);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, display->gl->ibo);
    if (glGetError())
        fprintf(stderr, "Error bind buffer\n");
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

    if (glGetError())
        fprintf(stderr, "Error draw elements\n");
    glFinish();
    eglSwapBuffers(display->egl->display, display->egl->surface);

    gst_buffer_unref(buffer);

}

static void *renderLoop(void* data) {
    struct display* display = (struct display*)data;
    display->egl = initEGL(display);
    display->gl = initGL(display);
    display->pending = TRUE;
    while (1) {
        while (!display->pending);
        draw(display);
        if (display->eos) {
            fprintf(stderr, "exit render loop\n");
            break;
        }
    }
}

int main(int argc, char* argv[]) {
    struct display* display = initWayland();
    pthread_t renderThread;
    GstElement* pipeline;
    display->ringBuff = initRingBuff();
    display->eos = FALSE;

    // Launch render thread
    if (pthread_create(&renderThread, NULL, renderLoop, display)) {
        fprintf(stderr, "Failed to create render thread\n");
        return -1;
    }

    while (!display->pending);

    // Create a GLib main loop to handle GStreamer events
    gst_init(&argc, &argv);
    pipeline = setupPipeline(display);
    gst_element_set_state(pipeline, GST_STATE_PLAYING);
    display->main_loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(display->main_loop);

    // Clean up and release resources
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
    g_main_loop_unref(display->main_loop);

    return 0;
}
