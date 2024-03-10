cmake_minimum_required(VERSION 3.25)
set(CMAKE_CXX_FLAGS "-Wall -g")

project(Staisp)

add_subdirectory(third_party/googletest)

include_directories(${CMAKE_CURRENT_LIST_DIR}/include)
aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/src STAISP_SRC)
add_executable(Staisp ${STAISP_SRC})
