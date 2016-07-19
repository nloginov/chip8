cmake_minimum_required(VERSION 3.4)

include_directories(${CONTRIB_DIR})

add_executable(runTests test_utils.cpp)
target_link_libraries(runTests ${CONTRIB_DIR}/libgtest.a ${CONTRIB_DIR}/libgtest_main.a pthread)