cmake_minimum_required( VERSION 3.16.3 )

project(OpenCL)

find_library(CL_LIB OpenCL ${CMAKE_ROOT_PATH}/usr/lib)

add_executable(vadd vadd.cpp)
target_link_libraries(vadd
  ${CL_LIB}
)

add_executable(device_info deviceInfo.cpp)
target_link_libraries(device_info
  ${CL_LIB}
)

add_executable(matrix_mul matrixMul.cpp)
target_link_libraries(matrix_mul
  ${CL_LIB}
)
