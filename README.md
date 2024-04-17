# GFX_PlayGround
Here is a play ground for APIs for GPU(OpenGL, Vulkan, OpenCL...)
General purpose is to learn APIs and how runs them on GPU. Secondary purpose is to implement abstraction layer for multi platform.

# Abstraction layer
Context and display classes are for this purpose.
## Display
Display class maintains the display resources to output the graphics buffer for actual panel. Allocated resources by display class is provided to context class to allocate graphics buffers.
Currently display classes for dispmanx, wayland and drm are implemented.

## Context
Context class is the rendering context for rendering applications. Context provides the rendering resources like surface, display and window.
Currently class for EGL is implemented.
