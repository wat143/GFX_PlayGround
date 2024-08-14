#include <VkContext.h>
#include <cstring>
#include <iostream>

VkContext::VkContext(NativeDisplay* disp, int api):Context(disp, api) {
    _surface = nullptr;
    _instance = nullptr;
}

VkContext::~VkContext() {
    vkDestroySurfaceKHR(_instance, _surface, nullptr);
    vkDestroyInstance(_instance, nullptr);
};

void* VkContext::getWindow() {
    return Disp->getNativeWindow();
}

void* VkContext::getDisplay() {
    return Disp->getNativeDisplay();
}

void* VkContext::getSurface() {
    VkResult err;
    /* ToDo: Support other window framework than wayland */
    PFN_vkCreateWaylandSurfaceKHR vkCreateWaylandSurfaceKHR;
    if (!_instance) {
        std::cerr << "Instance is not set yet " << std::endl;
        return nullptr;
    }

    vkCreateWaylandSurfaceKHR =
        (PFN_vkCreateWaylandSurfaceKHR)vkGetInstanceProcAddr(_instance, "vkCreateWaylandSurfaceKHR");

    if (!vkCreateWaylandSurfaceKHR)
        std::cerr << "Failed to acquire vkCreateWaylandSurfaceKHR\n";

    if (!_surface) {
        VkWaylandSurfaceCreateInfoKHR sci;
        memset(&sci, 0, sizeof(sci));
        sci.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
        sci.display = static_cast<wl_display*>(Disp->getNativeDisplay());
        sci.surface = static_cast<wl_surface*>(Disp->getSurface());
        err = vkCreateWaylandSurfaceKHR(_instance, &sci, nullptr, &_surface);
        if (err)
            std::cerr << "Failed to acquire VkSurfaceKHR\n";
    }
    return static_cast<void*>(_surface);
}

void VkContext::setInstance(void* instance) {
    _instance = static_cast<VkInstance>(instance);
}

int VkContext::makeCurrent() {
    /* NOP */
    return 1;
}
int VkContext::swapBuffers() {
    /* NOP */
    return 1;
}
