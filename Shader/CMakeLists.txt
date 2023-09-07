cmake_minimum_required( VERSION 3.16.3 )

project(Shader)

add_library(Shader STATIC GLShader.cpp)
target_include_directories(Shader
  PRIVATE
  ${GFX_INCLUDE_PATH}
)
set(BUILD_FW CACHE STRING "Build framework app running on")
if(BUILD_FW MATCHES DISPMANX)
  target_link_libraries(Shader
    brcmGLESv2)
else()
  target_link_libraries(Shader
    GLESv2)
endif()

