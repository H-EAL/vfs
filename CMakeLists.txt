cmake_minimum_required(VERSION 3.12)

# Set the project name
project(vfs)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

file(GLOB_RECURSE test_source_files CONFIGURE_DEPENDS "tests/*.cpp")

include_directories(include)
include_directories(include/vfs)

add_compile_definitions(_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING)

add_executable(vfs
        ${test_source_files})

# Link with lrt and pthread
set(CMAKE_EXE_LINKER_FLAGS "-lrt -pthread")

# This facilitates debugging in Clion (Posix).
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_GLIBCXX_DEBUG")