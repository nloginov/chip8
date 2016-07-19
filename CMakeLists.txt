cmake_minimum_required(VERSION 3.4)
project (CHIP8)

set(CONTRIB_DIR "${PROJECT_SOURCE_DIR}/contrib")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules" ${CMAKE_MODULE_PATH})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -std=c++1y")
set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")

include_directories(${SRC_DIR})

add_subdirectory (src)
add_subdirectory (tests)