#ifndef __VK_CONTEXT__
#define __VK_CONTEXT__

#include <vulkan/vulkan.h>
#include "Context.h"
#include "Display.h"

#include <vulkan/vulkan_wayland.h>

class VkContext : public Context {
private:
    typedef VkResult (*PFN_vkCreateWaylandSurfaceKHR)(VkInstance,const VkWaylandSurfaceCreateInfoKHR*,const VkAllocationCallbacks*,VkSurfaceKHR*);
    VkSurfaceKHR _surface;
    VkInstance _instance;
public:
    VkContext(NativeDisplay* disp, int api);
    ~VkContext();
    void* getWindow();
    void* getDisplay();
    void* getSurface();
    void setInstance(void* instance);
    int makeCurrent();
    int swapBuffers();
};

#endif
