cmake_minimum_required( VERSION 3.16.3 )

project(Context)

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
  add_compile_option(-DDISPMANX)
endif()
if(BUILD_FW MATCHES DRM)
  set(CONTEXT_SOURCE
    ContextSingleton.cpp
    DrmDisplay.cpp
    EglContext.cpp
    ContextFactory.cpp)
  set(CONTEXT_LIB
    drm
    gbm)
elseif()
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
