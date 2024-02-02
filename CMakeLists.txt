cmake_minimum_required( VERSION 3.16.3 )

project(GFX_PlayGround)

set(BUILD_FW CACHE STRING "Build framework app running on")
if(BUILD_FW MATCHES DISPMANX)
  link_directories(/opt/vc/src/hello_pi/libs)
endif()

set(VULKAN_APP CACHE STRING "Enable vulkan sample app build")
if(VULKAN_APP MATCHES ON)
    set(VULKAN_BUILD 1)
endif()

find_package(PkgConfig REQUIRED)

add_subdirectory(Context)
add_subdirectory(Mesh)
add_subdirectory(Renderer)
add_subdirectory(Resources)
