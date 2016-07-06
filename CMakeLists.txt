cmake_minimum_required(VERSION 3.4)
project(chip8)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules" ${CMAKE_MODULE_PATH})

find_package(SFML 2 REQUIRED system window graphics network audio)

set(BOOST_ROOT /usr/local/Cellar/boost/1.59.0)
find_package(Boost 1.59 REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -std=c++1y")

set(SOURCE_FILES main.cpp chip8.cpp opcode/parser.cpp opcode/types.h utils/bitutils.h)


add_executable(chip8 ${SOURCE_FILES})

include_directories(${Boost_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR})

if(SFML_FOUND)
    include_directories(${SFML_INCLUDE_DIR})
    target_link_libraries(chip8 ${SFML_LIBRARIES})
endif()
target_link_libraries(chip8 ${Boost_LIBRARIES})