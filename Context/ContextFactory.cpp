#include <iostream>
#include "ContextFactory.h"
#include "EglContext.h"
#include "DrmDisplay.h"
#include "Utils.h"

#ifdef DISPMANX
#include "DispmanxDisplay.h"
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
    else if (fw_type == DRM)
        disp = new DrmDisplay(fw_type);

    if (!disp)
        std::cerr << "Invalid FW type\n";
    Context* context = new EglContext(disp, OpenGLESv2);
    return context;
}
