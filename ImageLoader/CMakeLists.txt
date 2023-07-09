cmake_minimum_required( VERSION 3.16.3 )

project(GFX_PlayGround)

add_library(ImageLoader STATIC ImageLoader.cpp)
target_include_directories(ImageLoader
  PRIVATE
  ${CMAKE_SYSROOT}/include
  ${PROJECT_INCLUDE_PATH}
)
