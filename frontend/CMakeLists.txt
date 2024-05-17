
include_directories(${CMAKE_CURRENT_LIST_DIR}/../include)
include_directories(${CMAKE_CURRENT_LIST_DIR})

aux_source_directory(${CMAKE_CURRENT_LIST_DIR} SRC)

add_executable(SysYFrontend ${SRC})
target_link_libraries(SysYFrontend staisp)
