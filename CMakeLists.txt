cmake_minimum_required( VERSION 3.16.3 )

project(GFX_PlayGround)

set(BUILD_FW CACHE STRING "Build framework app running on")
if(BUILD_FW MATCHES DISPMANX)
  set(GFX_PATH "/opt/vc")
  link_directories(${GFX_PATH}/src/hello_pi/libs)
elseif(BUILD_FW MATCHES DRM)
  set(GFX_PATH "/usr/include/libdrm")
endif()

set(VULKAN_APP CACHE STRING "Enable vulkan sample app build")
if(VULKAN_APP MATCHES ON)
    set(VULKAN_BUILD 1)
endif()

find_package(PkgConfig REQUIRED)

set(CMAKE_SYSROOT $ENV{SYSROOT})
set(GFX_INCLUDE_PATH ${GFX_PATH}/include)
set(GFX_LIBRARY_PATH ${GFX_PATH}/lib)

link_directories(${GFX_LIBRARY_PATH})

add_subdirectory(ImageLoader)
add_subdirectory(Shader)
add_subdirectory(Context)
add_subdirectory(Mesh)
add_subdirectory(Object)
add_subdirectory(Renderer)
add_subdirectory(Resources)
