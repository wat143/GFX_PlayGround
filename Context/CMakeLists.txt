cmake_minimum_required( VERSION 3.16.3 )

project(Context)

pkg_check_modules(LIBEGL REQUIRED egl>=1.0.0)
pkg_check_modules(LIBGLESV2 REQUIRED glesv2>=2.0.0)
if(BUILD_FW MATCHES DISPMANX)
  set(CONTEXT_SOURCE
    ContextSingleton.cpp
    DispmanxDisplay.cpp
    EglContext.cpp
    ContextFactory.cpp)
  set(CONTEXT_LIB
    brcmEGL
    brcmGLESv2
    bcm_host)
  add_compile_options(-DDISPMANX=1)
elseif(BUILD_FW MATCHES DRM)
  set(CONTEXT_SOURCE
    ContextSingleton.cpp
    DrmDisplay.cpp
    EglContext.cpp
    ContextFactory.cpp)
  set(CONTEXT_LIB
    EGL
    GLESv2
    drm
    gbm)
  add_compile_options(-DDRM=1)
elseif(BUILD_FW MATCHES WAYLAND)
  set(CONTEXT_SOURCE
    xdg-shell.c
    ContextSingleton.cpp
    WaylandDisplay.cpp
    EglContext.cpp
    ContextFactory.cpp)
  set(CONTEXT_LIB
    EGL
    GLESv2
    wayland-client
    wayland-egl)
  add_compile_options(-DWAYLAND=1)
else()
  message(FATAL, "No FW is set")
endif()

add_library(Context STATIC ${CONTEXT_SOURCE})
target_include_directories(Context
  PRIVATE
  ${CMAKE_SOURCE_DIR}/Context
  ${CMAKE_SOURCE_DIR}/ImageLoader
  ${CMAKE_SOURCE_DIR}/Mesh
  ${CMAKE_SOURCE_DIR}/Shader
  ${CMAKE_SOURCE_DIR}/include
  /usr/include/libdrm
  ${GFX_INCLUDE_PATH}
)
target_link_libraries(Context ${CONTEXT_LIB})
