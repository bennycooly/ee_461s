cmake_minimum_required(VERSION 3.6)
project(yash)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c99")

set(SOURCE_FILES yash.c)
add_executable(yash ${SOURCE_FILES})
