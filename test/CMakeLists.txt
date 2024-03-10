include(GoogleTest)
include_directories(third_party/googletest/googletest/include/)

add_executable(StaispTest test.cpp)
target_link_libraries(StaispTest gtest gtest_main)

gtest_discover_tests(StaispTest)
