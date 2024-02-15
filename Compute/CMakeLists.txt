cmake_minimum_required( VERSION 3.16.3 )

project(OpenCL)

find_library(MyLibs OpenCL ${CMAKE_ROOT_PATH}/usr/lib)

add_executable(vadd vadd.cpp)
target_link_libraries(vadd
  ${MyLibs}
)

add_executable(device_info deviceInfo.cpp)
target_link_libraries(device_info
  ${MyLibs}
)
