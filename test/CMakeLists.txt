include(GoogleTest)
include_directories(${CMAKE_CURRENT_LIST_DIR}/../third_party/googletest/googletest/include/)
include_directories(${CMAKE_CURRENT_LIST_DIR}/../include)
include_directories(${CMAKE_CURRENT_LIST_DIR})

add_executable(FrontendTest test_frontend.cpp test_sysy_frontend.cpp)
target_link_libraries(FrontendTest gtest gtest_main staisp)
gtest_discover_tests(FrontendTest)


add_executable(BackendTest test_backend.cpp test_sysy_backend.cpp)
target_link_libraries(BackendTest gtest gtest_main staisp)
gtest_discover_tests(BackendTest)
