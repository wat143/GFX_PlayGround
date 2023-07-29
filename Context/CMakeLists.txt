cmake_minimum_required( VERSION 3.16.3 )

project(Context)

if(BUILD_FW MATCHES PI3)
  set(CONTEXT_SOURCE
    ContextSingleton.cpp
    DispmanxDisplay.cpp
    EglContext.cpp
    PiContextFactory.cpp)
  set(CONTEXT_LIB
    brcmEGL
    brcmGLESv2
    bcm_host)
elseif()
  message(FATAL, "No FW is set")
endif()

add_library(Context STATIC ${CONTEXT_SOURCE})
target_include_directories(Context
  PRIVATE
  ${CMAKE_SOURCE_DIR}/Context
  ${CMAKE_SOURCE_DIR}/ImageLoader
  ${CMAKE_SOURCE_DIR}/Mesh
  ${CMAKE_SOURCE_DIR}/Shader
  ${CMAKE_SOURCE_DIR}/include
  ${GFX_INCLUDE_PATH}
)
target_link_libraries(Context ${CONTEXT_LIB})
