cmake_minimum_required(VERSION 3.5)

project(MinimizePath)

set(BUILD_SHARED_LIBS OFF)

include_directories("raylib/src/")
include_directories("raygui/src")

add_subdirectory(main)
add_subdirectory(raylib)
