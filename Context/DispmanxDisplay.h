#ifndef __DISPMANXDISPLAY__
#define __DISPMANXDISPLAY__

#include "Display.h"
#include <EGL/egl.h>
#include <EGL/eglext.h>

class DispmanxDisplay : public NativeDisplay {
private:
    EGL_DISPMANX_WINDOW_T nativewindow;
    DISPMANX_ELEMENT_HANDLE_T dispman_element;
    DISPMANX_DISPLAY_HANDLE_T dispman_display;
    DISPMANX_UPDATE_HANDLE_T dispman_update;
    VC_RECT_T dst_rect;
    VC_RECT_T src_rect;
public:
    DispmanxDisplay(int type);
    ~DispmanxDisplay();
    void* getNativeDisplay();
    void* getNativeWindow();
    void* getDisplayDev();
    int pageFlip() { return 0; }
};

#endif
