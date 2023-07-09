cmake_minimum_required( VERSION 3.16.3 )

project(GFX_PlayGround)

set(CMAKE_SYSROOT $ENV{SYSROOT})
set(GFX_INCLUDE_PATH $ENV{GFX_PATH}/include)
set(GFX_LIBRARY_PATH $ENV{GFX_PATH}/lib)

link_directories(${GFX_LIBRARY_PATH})
link_directories(${GFX_PATH}/src/hello_pi/libs)
add_subdirectory(ImageLoader)
add_subdirectory(Shader)
add_subdirectory(Context)
add_subdirectory(Mesh)
add_subdirectory(Object)
add_subdirectory(Renderer)
