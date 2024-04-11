#include <iostream>
#include <cassert>
#include "EglContext.h"
#include "EGL/egl.h"
#include "EGL/eglext.h"
#include "Utils.h"

EglContext::EglContext(NativeDisplay* disp, int api):Context(disp, api){
    int ret = -1;
    EGLint num_config = 0;
    EGLConfig config;
    const EGLint attribute_list[] =
        {
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_RED_SIZE, 1,
            EGL_GREEN_SIZE, 1,
            EGL_BLUE_SIZE, 1,
            EGL_ALPHA_SIZE, 0,
            EGL_DEPTH_SIZE, 1,
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
            EGL_NONE
        };

    const EGLint context_attributes[] = 
        {
            EGL_CONTEXT_CLIENT_VERSION, 2,
            EGL_NONE
        };

    nativeDisp = disp;
    fwType = disp->getFWType();
    // Get eglDisplay
    void* dev = disp->getDisplayDev();
    if (!dev)
        display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    else
        display = eglGetDisplay(static_cast<EGLNativeDisplayType>(dev));
    assert(display != EGL_NO_DISPLAY);

    // Initialize the EGL display connection
    ret = eglInitialize(display, nullptr, nullptr);
    assert(ret != EGL_FALSE);

    // Get an appropriate EGL frame buffer configuration
    if (fwType == Drm)
        ChooseConfig(display, attribute_list, static_cast<EGLint>(disp->getFormat()), &config);
    else
        ChooseConfig(display, attribute_list, 0, &config);
    // Bind GLESv2
    if (API == OpenGLESv2)
        ret = eglBindAPI(EGL_OPENGL_ES_API);
    else
        ret = EGL_FALSE;
    assert(ret != EGL_FALSE);
   
    // Create an EGL rendering context
    if (fwType == Drm)
        context =
            eglCreateContext(display, config, EGL_NO_CONTEXT, context_attributes);
    else
        context =
            eglCreateContext(display, config, EGL_NO_CONTEXT, context_attributes);
    assert(context != EGL_NO_CONTEXT);

    // Create an EGL window surface
    surface = eglCreateWindowSurface(display,
                                     config,
                                     reinterpret_cast<EGLNativeWindowType>(disp->getNativeWindow()),
                                     nullptr);
    assert(surface != EGL_NO_SURFACE);
    eglMakeCurrent(display, surface, surface, context);
}

EglContext::~EglContext(){
    // Clear Display state
    if (fwType != Drm)
        eglSwapBuffers(display, surface);

    // Release resources
    eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(display, context);
    eglTerminate(display );
}

void* EglContext::getDisplay() { return (void*)&display; }

void* EglContext::getSurface() { return (void*)&surface; }

void* EglContext::getWindow() { return nullptr; }

int EglContext::makeCurrent() {
    int ret;
    ret = eglMakeCurrent(display, surface, surface, context);
    return ret;
}

int EglContext::swapBuffers() {
    int ret;
    if (fwType == Drm) {
        ret = eglSwapBuffers(display, surface);
        nativeDisp->pageFlip();
    }
    else
        ret = eglSwapBuffers(display, surface);
    return ret;
}

void EglContext::ChooseConfig(EGLDisplay display, const EGLint* attribs, EGLint visualID, EGLConfig *configOut) {
    EGLint count = 0, matched = 0, ret;
    EGLConfig *configs;
    int configIdx = -1;

    ret = eglGetConfigs(display, NULL, 0, &count);
    assert(ret != EGL_FALSE && count > 0);
    configs = new EGLConfig[count];
    ret = eglChooseConfig(display, attribs, configs, count, &matched);
    assert(ret != EGL_FALSE);
    if (!visualID)
        configIdx = 0;
    if (configIdx == -1) {
        for (int i = 0; i < count; i++) {
            EGLint id;
            if (!eglGetConfigAttrib(display, configs[i], EGL_NATIVE_VISUAL_ID, &id))
                continue;
            if (visualID == id) {
                configIdx = i;
                break;
            }
        }
    }
    if (configIdx != -1) {
        std::cout << "Found EGL config: " << configIdx << std::endl;
        *configOut = configs[configIdx];
    }
    delete[] configs;
    assert(configIdx != -1);
}
