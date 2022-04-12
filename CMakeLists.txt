cmake_minimum_required(VERSION 3.21)
project(Monitory)

set(CMAKE_CXX_STANDARD 14)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/magazine.txt
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

set(GCC_COVERAGE_COMPILE_FLAGS "-pthread")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}" )

add_executable(Monitory main.h main.cpp)
