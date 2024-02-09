cmake_minimum_required( VERSION 3.16.3 )

project(Renderer)

if(BUILD_FW MATCHES DISPMANX)
  add_compile_option(-DDISPMANX)
  set(DISPMANX_LIBS
      brcmEGL
      brcmGLESv2
)
elseif(BUILD_FW MATCHES DRM)
  add_compile_options(-DNATIVE_DISP_DRM)
elseif(BUILD_FW MATCHES WAYLAND)
  add_compile_options(-DNATIVE_DISP_WAYLAND)
endif()

add_executable(simple_teapot simple_teapot.cpp)
target_include_directories(simple_teapot
  PRIVATE
  ${CMAKE_SOURCE_DIR}/Context
  ${CMAKE_SOURCE_DIR}/Mesh
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/glm
  ${GFX_INCLUDE_PATH}/../src/hello_pi/libs/revision
)

target_link_libraries(simple_teapot
  assimp
  Mesh
  Context
)

if(BUILD_FW MATCHES DISPMANX)
  target_link_libraries(simple_teapot
    DISPMANX_LIBS
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
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/glm
  ${GFX_INCLUDE_PATH}/../src/hello_pi/libs/revision
)

target_link_libraries(simple_triangle
  Context
)

if(BUILD_FW MATCHES DISPMANX)
  target_link_libraries(simple_triangle
        DISPMANX_LIBS
)
else()
  target_link_libraries(simple_teapot
    EGL
    GLESv2
)
endif()

add_executable(simple_alpha alphaBlend.cpp)
target_include_directories(simple_alpha
  PRIVATE
  ${CMAKE_SOURCE_DIR}/Context
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/glm
  ${GFX_INCLUDE_PATH}/../src/hello_pi/libs/revision
)

target_link_libraries(simple_alpha
  Context
)

if(BUILD_FW MATCHES DISPMANX)
  target_link_libraries(simple_alpha
        DISPMANX_LIBS
)
else()
  target_link_libraries(simple_alpha
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
foreach(DIR ${GSTREAMER_INCLUDE_DIRS})
    file(TO_CMAKE_PATH "${CMAKE_SYSROOT}/${DIR}" GST_DIR)
    list(APPEND GST_INCLUDE_DIRS ${GST_DIR})
endforeach()
target_include_directories(videoStream
  PRIVATE
  ${CMAKE_SOURCE_DIR}/Context
  ${CMAKE_SOURCE_DIR}/include
  ${GST_INCLUDE_DIRS}
)

target_link_libraries(videoStream
  pthread
  gstreamer-1.0
  gstbase-1.0
  gstapp-1.0
  gobject-2.0
  glib-2.0
  EGL
  GLESv2
  wayland-client
  wayland-egl
)

if(VULKAN_BUILD MATCHES 1)
# Build shader files
set(SHADER_SRC ${CMAKE_SOURCE_DIR}/Renderer/shaders)
set(SHADER_DST ${CMAKE_BINARY_DIR}/Renderer/shaders)
file(MAKE_DIRECTORY ${SHADER_DST})
add_custom_command(
    OUTPUT ${SHADER_DST}/vert.spv
    COMMAND glslc ${SHADER_SRC}/shader.vert -o ${SHADER_DST}/vert.spv
    DEPENDS ${SHADER_SRC}/shader.vert
)
add_custom_target(VertShader DEPENDS ${SHADER_DST}/vert.spv)

add_custom_command(
    OUTPUT ${SHADER_DST}/vert3d.spv
    COMMAND glslc ${SHADER_SRC}/shader3d.vert -o ${SHADER_DST}/vert3d.spv
    DEPENDS ${SHADER_SRC}/shader3d.vert
)
add_custom_target(VertShader3D DEPENDS ${SHADER_DST}/vert3d.spv)

add_custom_command(
    OUTPUT ${SHADER_DST}/frag.spv
    COMMAND glslc ${SHADER_SRC}/shader.frag -o ${SHADER_DST}/frag.spv
    DEPENDS ${SHADER_SRC}/shader.frag
)
add_custom_target(FragShader DEPENDS ${SHADER_DST}/frag.spv)

add_executable(vulkanHelloTriangle vulkanHelloTriangle.cpp)
add_compile_definitions(NDEBUG)
add_dependencies(vulkanHelloTriangle VertShader FragShader)

target_link_libraries(vulkanHelloTriangle
    vulkan
    glfw
)

target_include_directories(vulkanHelloTriangle
  PRIVATE
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/glm
)


add_executable(simple_teapot_vk simple_teapot_vk.cpp)
add_compile_definitions(NDEBUG)
add_dependencies(simple_teapot_vk VertShader3D FragShader)

target_include_directories(simple_teapot_vk
  PRIVATE
  ${CMAKE_SOURCE_DIR}/Mesh
  ${CMAKE_SOURCE_DIR}/glm
)

target_link_libraries(simple_teapot_vk
    vulkan
    glfw
    assimp
    Mesh
)

endif()
