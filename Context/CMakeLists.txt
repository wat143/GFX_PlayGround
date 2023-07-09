cmake_minimum_required( VERSION 3.16.3 )

project(GFX_PlayGround)

add_library(Context STATIC
  ContextSingleton.cpp
  DispmanxDisplay.cpp
  EglContext.cpp
  PiContextFactory.cpp
)
target_include_directories(Context
  PRIVATE
  ${CMAKE_SOURCE_DIR}/Context
  ${CMAKE_SOURCE_DIR}/ImageLoader
  ${CMAKE_SOURCE_DIR}/Mesh
  ${CMAKE_SOURCE_DIR}/Shader
  ${CMAKE_SOURCE_DIR}/include
  ${GFX_INCLUDE_PATH}
)
target_link_libraries(Context
  brcmEGL
  brcmGLESv2
  bcm_host
)
