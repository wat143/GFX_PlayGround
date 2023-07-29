cmake_minimum_required( VERSION 3.16.3 )

project(Shader)

add_library(Shader STATIC GLShader.cpp)
target_include_directories(Shader
  PRIVATE
  ${GFX_INCLUDE_PATH}
)
target_link_libraries(Shader
  brcmGLESv2
)
