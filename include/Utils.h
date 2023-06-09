#ifndef __UTILS__
#define __UTILS__

// Platform type
enum PlatformType {
      RaspPi3 = 1,
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
