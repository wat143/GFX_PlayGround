#include <iostream>
#include "ContextFactory.h"
#include "EglContext.h"

#include "Utils.h"

#ifdef DISPMANX
#include "DispmanxDisplay.h"
#endif
#ifdef DRM
#include "DrmDisplay.h"
#endif
#ifdef WAYLAND
#include "WaylandDisplay.h"
#endif

Context* ContextFactory::create(int fw_type) {
    NativeDisplay* disp = nullptr;
    if (fw_type == DispmanX) {
#ifdef DISPMANX
        disp = new DispmanxDisplay(fw_type);
#else
        /* NOP */
        ;
#endif
    }
    else if (fw_type == Drm || fw_type == Wayland) {
#ifdef DRM
        disp = new DrmDisplay(fw_type);
#endif
#ifdef WAYLAND
        disp = new WaylandDisplay(fw_type);
#else
        /* NOP */
        ;
#endif
    }
    if (!disp)
        std::cerr << "Invalid FW type\n";
    Context* context = new EglContext(disp, OpenGLESv2);
    return context;
}
