cmake_minimum_required(VERSION 3.25)

project(Staisp)
set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)

include_directories(${CMAKE_CURRENT_LIST_DIR}/include)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/src STAISP_SRC)
add_executable(Staisp ${STAISP_SRC})
add_definitions(-Wall -g)