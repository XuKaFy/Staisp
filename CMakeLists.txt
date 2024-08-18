cmake_minimum_required(VERSION 3.21)

project(Staisp)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g -std=c++17")

enable_testing()

add_subdirectory(third_party/googletest)
add_subdirectory(test)

include_directories(${PROJECT_SOURCE_DIR}/include)
aux_source_directory(${PROJECT_SOURCE_DIR}/src STAISP_SRC)
aux_source_directory(${PROJECT_SOURCE_DIR}/frontend STAISP_FE)
aux_source_directory(${PROJECT_SOURCE_DIR}/include STAISP_INC)

add_custom_target(clean-test rm ${CMAKE_BINARY_DIR}/test/sysy_tests_*)
add_executable(bbyyy ${STAISP_SRC} ${STAISP_FE} ${STAISP_INC})