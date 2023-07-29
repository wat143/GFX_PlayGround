cmake_minimum_required( VERSION 3.16.3 )

project(Object)

if(BUILD_FW MATCHES PI3)
  set(OBJECT_SOURCE
    PiGLObject.cpp)
elseif()
  message(FATAL, "No FW is set")
endif()

add_library(Object STATIC ${OBJECT_SOURCE})
include_directories(Object
  PRIVATE
  ${CMAKE_SOURCE_DIR}/
  ${CMAKE_SOURCE_DIR}/Context
  ${CMAKE_SOURCE_DIR}/ImageLoader
  ${CMAKE_SOURCE_DIR}/Mesh
  ${CMAKE_SOURCE_DIR}/Object
  ${CMAKE_SOURCE_DIR}/Shader
  ${CMAKE_SOURCE_DIR}/include
  ${GFX_INCLUDE_PATH}
)
target_link_libraries(Object
  ImageLoader
  Mesh
  Shader
  Context
)
