cmake_minimum_required( VERSION 3.16.3 )

project(GFX_PlayGround)

set(BUILD_FW CACHE STRING "Build framework app running on")
if(BUILD_FW MATCHES PI3)
  set(GFX_PATH "/opt/vc")
  link_directories(${GFX_PATH}/src/hello_pi/libs)
elseif()
  message(FATAL, "No FW is set")
endif()

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
