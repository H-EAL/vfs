#define CATCH_CONFIG_MAIN
#include "catch_amalgamated.hpp"

#include <filesystem>
#include <iostream>
#include <fstream>
#include <unordered_set>

#include "vfs.hpp"
#include "vfs/logging.hpp"

const std::string text =
"Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. "
"Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat."
" Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. "
"Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.";

const std::string text2 =
"----- ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. "
"Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat."
" Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. "
"Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.";

#include "directory_tests.hpp"

#include "file_tests.hpp"

#include "file_view_tests.hpp"

#include "move_tests.hpp"

#include "shared_memory_tests.hpp"

#include "watcher_tests.hpp"

TEST_CASE("Clean up.", "[cleanup]")
{
    std::filesystem::remove_all(".\\foo");
    std::filesystem::remove_all(".\\test");
}

// TODO : Virtual Array, Pipe, Virtual Allocator tests.
