cmake_minimum_required( VERSION 3.16.3 )

project(Renderer)

if(BUILD_FW MATCHES DISPMANX)
  add_compile_option(-DDISPMANX)
elseif(BUILD_FW MATCHES DRM)
  add_compile_options(-DDRM=1)
elseif(BUILD_FW MATCHES WAYLAND)
  add_compile_options(-DWAYLAND=1)
endif()

add_executable(simple_teapot simple_teapot.cpp)
target_include_directories(simple_teapot
  PRIVATE
  ${CMAKE_SOURCE_DIR}/Context
  ${CMAKE_SOURCE_DIR}/ImageLoader
  ${CMAKE_SOURCE_DIR}/Mesh
  ${CMAKE_SOURCE_DIR}/Object
  ${CMAKE_SOURCE_DIR}/Shader
  ${CMAKE_SOURCE_DIR}/include
  ${GFX_INCLUDE_PATH}
  ${GFX_INCLUDE_PATH}/../src/hello_pi/libs/revision
)

target_link_libraries(simple_teapot
  ImageLoader
  assimp
  Mesh
  Context
  Shader
  Object
)

if(BUILD_FW MATCHES DISPMANX)
  target_link_libraries(simple_teapot
    brcmEGL
    brcmGLESv2
)
else()
  target_link_libraries(simple_teapot
    EGL
    GLESv2
)
endif()

add_executable(simple_triangle simple_triangle.cpp)
target_include_directories(simple_triangle
  PRIVATE
  ${CMAKE_SOURCE_DIR}/Context
  ${CMAKE_SOURCE_DIR}/ImageLoader
  ${CMAKE_SOURCE_DIR}/Mesh
  ${CMAKE_SOURCE_DIR}/Object
  ${CMAKE_SOURCE_DIR}/Shader
  ${CMAKE_SOURCE_DIR}/include
  ${GFX_INCLUDE_PATH}
  ${GFX_INCLUDE_PATH}/../src/hello_pi/libs/revision
)

target_link_libraries(simple_triangle
  ImageLoader
  assimp
  Mesh
  Context
  Shader
  Object
)

if(BUILD_FW MATCHES DISPMANX)
  target_link_libraries(simple_triangle
    brcmEGL
    brcmGLESv2
)
else()
  target_link_libraries(simple_teapot
    EGL
    GLESv2
)
endif()

add_executable(videoStream
  videoStream.c
  ${CMAKE_SOURCE_DIR}/Context/xdg-shell.c
)

pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0>=1.0.0)
pkg_check_modules(GSTBASE REQUIRED gstreamer-base-1.0>=1.0.0)
pkg_check_modules(GSTAPP REQUIRED gstreamer-app-1.0>=1.0.0)
pkg_check_modules(GOBJECT REQUIRED gobject-2.0>=1.0.0)
pkg_check_modules(GLIB REQUIRED glib-2.0>=1.0.0)
pkg_check_modules(WAYLAND_CLIENT REQUIRED wayland-client>=1.18.0)
pkg_check_modules(WAYLAND_EGL REQUIRED wayland-egl>=18.1.0)
pkg_check_modules(LIBEGL REQUIRED egl>=1.0.0)
pkg_check_modules(LIBGLESV2 REQUIRED glesv2>=2.0.0)

target_include_directories(videoStream
  PRIVATE
  ${CMAKE_SOURCE_DIR}/Context
  ${CMAKE_SOURCE_DIR}/include
  ${GSTREAMER_INCLUDE_DIRS}
)

target_link_libraries(videoStream
  ${GSTREAMER_LINK_LIBRARIES}
  ${GSTBASE_LINK_LIBRARIES}
  ${GSTAPP_LINK_LIBRARIES}
  ${GOBJECT_LINK_LIBRARIES}
  ${GLIB_LINK_LIBRARIES}
  ${LIBEGL_LINK_LIBRARIES}
  ${LIBGLESV2_LINK_LIBRARIES}
  ${WAYLAND_CLIENT_LINK_LIBRARIES}
  ${WAYLAND_EGL_LINK_LIBRARIES}
)
