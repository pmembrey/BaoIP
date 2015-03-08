cmake_minimum_required(VERSION 2.8)
project("BaoIP")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMake")

include_directories("${CMAKE_SOURCE_DIR}/include" "${CMAKE_SOURCE_DIR}/src/baoip/")

add_library(baoip SHARED src/baoip/baoip.c)
add_executable(stack stack.c src/helpers.c)

target_link_libraries (stack ${CMAKE_THREAD_LIBS_INIT} baoip)