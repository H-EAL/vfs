cmake_minimum_required(VERSION 3.12)

project(vfs)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_GLIBCXX_DEBUG")

add_executable(vfs_tests
        tests/catch_amalgamated.cpp
        tests/vfs_tests.cpp
)

target_include_directories(vfs_tests PRIVATE
        include
        include/vfs
)

# Link with lrt and pthread
set(CMAKE_EXE_LINKER_FLAGS "-lrt -pthread")
