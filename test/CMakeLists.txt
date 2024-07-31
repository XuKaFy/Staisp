include(GoogleTest)
include_directories(${CMAKE_CURRENT_LIST_DIR}/../third_party/googletest/googletest/include/)
include_directories(${CMAKE_CURRENT_LIST_DIR}/../include)
include_directories(${CMAKE_CURRENT_LIST_DIR})

add_executable(Test2024 test_frontend.cpp test_backend.cpp test_2024.cpp)
target_link_libraries(Test2024 gtest gtest_main staisp)
gtest_discover_tests(Test2024)