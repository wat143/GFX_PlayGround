cmake_minimum_required( VERSION 3.16.3 )

project(Resources)

# Set the source and destination directories
set(SOURCE_DIR "${CMAKE_SOURCE_DIR}/Resources")
set(DEST_DIR "${CMAKE_BINARY_DIR}")

# Copy files from the source directory to the build directory
file(COPY ${SOURCE_DIR} DESTINATION ${DEST_DIR})
