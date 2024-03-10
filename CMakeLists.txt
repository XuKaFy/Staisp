cmake_minimum_required(VERSION 3.25)
set(CMAKE_CXX_FLAGS "-Wall -g" CACHE STRING "Set C++ Compiler Flags" FORCE)

project(Staisp)

enable_testing()

add_subdirectory(third_party/googletest)
add_subdirectory(test)

include_directories(${CMAKE_CURRENT_LIST_DIR}/include)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/src STAISP_SRC)
add_executable(Staisp compiler.cpp ${STAISP_SRC})
add_library(staisp ${STAISP_SRC})
