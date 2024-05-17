cmake_minimum_required(VERSION 3.21)

project(Staisp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g -std=c++17")

enable_testing()

add_subdirectory(third_party/googletest)
add_subdirectory(test)
add_subdirectory(frontend)

include_directories(${CMAKE_CURRENT_LIST_DIR}/include)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/src STAISP_SRC)
add_executable(frontend frontend.cpp ${STAISP_SRC})
add_library(staisp ${STAISP_SRC})
