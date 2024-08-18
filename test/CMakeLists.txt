include(GoogleTest)
include_directories(${PROJECT_SOURCE_DIR}/third_party/googletest/googletest/include/)
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/test)

add_executable(Test2024 test_frontend.cpp test_backend.cpp test_2024.cpp)
target_link_libraries(Test2024 gtest gtest_main)
gtest_discover_tests(Test2024)