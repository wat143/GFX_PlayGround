cmake_minimum_required( VERSION 3.16.3 )

project(GFX_PlayGround)

find_package(PkgConfig REQUIRED)

set(BUILD_FW CACHE STRING "Build framework app running on")
if(BUILD_FW MATCHES DISPMANX)
  link_directories(/opt/vc/src/hello_pi/libs)
elseif(BUILD_FW MATCHES DRM)
  pkg_check_modules(LIBDRM REQUIRED libdrm>=2.4.0)
  pkg_check_modules(LIBGBM REQUIRED gbm>=1.0.0)
elseif(BUILD_FW MATCHES WAYLAND)
  pkg_check_modules(WAYLAND_CLIENT REQUIRED wayland-client>=1.18.0)
  pkg_check_modules(WAYLAND_CLIENT REQUIRED wayland-egl>=18.1.0)
endif()

set(VULKAN_APP CACHE STRING "Enable vulkan sample app build")
if(VULKAN_APP MATCHES ON)
    set(VULKAN_BUILD 1)
    pkg_check_modules(LIBVULKAN REQUIRED vulkan>=1.3.0)
    pkg_check_modules(LIBGLFW3 REQUIRED glfw3>=3.3.0)
endif()

add_subdirectory(Context)
add_subdirectory(Mesh)
add_subdirectory(Renderer)
add_subdirectory(Resources)
