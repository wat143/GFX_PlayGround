cmake_minimum_required( VERSION 3.16.3 )

project(GFX_PlayGround)

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
  brcmEGL
  brcmGLESv2
  Context
  Shader
  Object
)

