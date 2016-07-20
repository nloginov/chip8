cmake_minimum_required(VERSION 3.4)
project(chip8)

find_package(SFML 2 REQUIRED system window graphics network audio)

set(BOOST_ROOT /usr/local/Cellar/boost/1.59.0)
find_package(Boost 1.59 REQUIRED)

set(SOURCE_FILES chip8.cpp opcode/parser.cpp)

add_library(chip8lib ${SOURCE_FILES})

add_executable(chip8 main.cpp)
target_link_libraries(chip8 chip8lib)

include_directories(${Boost_INCLUDE_DIRS})

if(SFML_FOUND)
    include_directories(${SFML_INCLUDE_DIR})
    target_link_libraries(chip8lib ${SFML_LIBRARIES})
endif()
target_link_libraries(chip8 ${Boost_LIBRARIES})