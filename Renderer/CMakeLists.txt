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

pkg_check_modules(LIBVULKAN REQUIRED vulkan>=1.3.0)
pkg_check_modules(LIBGLFW3 REQUIRED glfw3>=3.3.0)

add_executable(vulkanHelloTriangle vulkanHelloTriangle.cpp)
add_compile_definitions(NDEBUG)
add_dependencies(vulkanHelloTriangle VertShader FragShader)

target_link_libraries(vulkanHelloTriangle
    vulkan
    glfw
)

add_executable(simple_teapot_vk simple_teapot_vk.cpp)
add_compile_definitions(NDEBUG)
add_dependencies(simple_teapot_vk VertShader3D FragShader)

target_include_directories(simple_teapot_vk
  PRIVATE
  ${CMAKE_SOURCE_DIR}/Mesh
)

target_link_libraries(simple_teapot_vk
    vulkan
    glfw
    assimp
    Mesh
)

endif()
