cmake_minimum_required( VERSION 3.16.3 )

project(Mesh)

pkg_check_modules(LIBASSIMP QUIET assimp>=5.0)
if(NOT LIBASSIMP_FOUND)
  OPTION( ASSIMP_BUILD_ASSIMP_TOOLS
    "If the supplementary tools for Assimp are built in addition to the library."
    OFF
  )
  OPTION ( ASSIMP_BUILD_TESTS
    "If the test suite for Assimp is built in addition to the library."
    OFF
  )
  add_subdirectory(assimp)
endif()

add_library(Mesh STATIC
  AssimpMesh.cpp
)
target_include_directories(Mesh
  PRIVATE
  ${PROJECT_INCLUDE_PATH}
)

target_link_libraries(Mesh assimp)
