cmake_minimum_required(VERSION 3.6)
project(yash)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=gnu99")

set(SOURCE_FILES yash.c line.h line.c pgroup.h pgroup.c pgroup-list.h pgroup-list.c process.h process.c session.h session.c)

add_executable(yash ${SOURCE_FILES})