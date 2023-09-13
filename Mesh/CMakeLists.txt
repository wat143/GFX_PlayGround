cmake_minimum_required( VERSION 3.16.3 )

project(Mesh)

pkg_check_modules(LIBASSIMP REQUIRED assimp>=5.0)

add_library(Mesh STATIC
  AssimpMesh.cpp
)
target_include_directories(Mesh
  PRIVATE
  ${PROJECT_INCLUDE_PATH}
)

target_link_libraries(Mesh assimp)
