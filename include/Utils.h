#ifndef __UTILS__
#define __UTILS__

// Framework type
enum FrameworkType {
      DispmanX = 1,
      DRM
};

// API
enum APIType {
      OpenGLESv2 = 1,
      Vulkan
};

// Attribute type
enum AttrType {
      VERTEX_DATA = 1,
      VERTEX_COLOR,
      VERTEX_NORMAL,
      UV_DATA,
      ATTR_TYPE_MAX
};

#endif
