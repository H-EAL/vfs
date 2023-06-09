cmake_minimum_required(VERSION 3.12)

project(vfs)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_GLIBCXX_DEBUG")

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}" CACHE PATH "..." FORCE)
endif()

##############################vfs_tests##############################
add_executable(vfs_tests tests/vfs_tests.cpp tests/catch_amalgamated.cpp)

target_include_directories(vfs_tests PRIVATE include include/vfs)

install(TARGETS vfs_tests DESTINATION bin)
