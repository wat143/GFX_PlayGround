#include "ContextFactory.h"
#include "EglContext.h"
#include "DispmanxDisplay.h"
#include "DrmDisplay.h"
#include "Utils.h"

Context* ContextFactory::create(int fw_type) {
    Display* disp = nullptr;
    if (fw_type == DispmanX)
        disp = new DispmanxDisplay(fw_type);
    else if (fw_type == DRM)
        disp = new DrmDisplay(fw_type);
    else
        std::cerr << "Invalid FW type\n";
    Context* context = new EglContext(disp, OpenGLESv2);
    return context;
}
