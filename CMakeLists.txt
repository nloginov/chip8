cmake_minimum_required(VERSION 3.4)
project(chip8)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -std=c++1y")

set(SOURCE_FILES main.cpp chip8.cpp)
add_executable(chip8 ${SOURCE_FILES})