include(GoogleTest)
include_directories(${CMAKE_CURRENT_LIST_DIR}/../third_party/googletest/googletest/include/)
include_directories(${CMAKE_CURRENT_LIST_DIR}/../include)

aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/src TEST_SRC)

add_executable(StaispTest ${TEST_SRC})
target_link_libraries(StaispTest gtest gtest_main staisp)

gtest_discover_tests(StaispTest)
