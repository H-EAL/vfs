#define CATCH_CONFIG_MAIN
#include "catch_amalgamated.hpp"

#include <filesystem>
#include <fstream>
#include <unordered_set>

#include "vfs.hpp"
#include "vfs/logging.hpp"

// Change test working directory here (without a trailing slash).
// Make sure to ONLY use the directory separator / and not \\. More information in clean up test case below.
const std::string test_directory = ".";

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

TEST_CASE("Setup.", "[cleanup]")
{
    // Note that std::filesystem::path does not sanitize paths (and make them cross platform) like vfs::path.
    // Make sure to use / as directory separator in a std::filesystem::path, if you use \\ this will not work in posix.
    std::filesystem::remove_all(test_directory + "/foo");
    std::filesystem::remove_all(test_directory + "/test");
}

#include "directory_tests.hpp"

#include "file_tests.hpp"

#include "file_view_tests.hpp"

#include "move_tests.hpp"

#include "shared_memory_tests.hpp"

#include "watcher_tests.hpp"

TEST_CASE("Teardown.", "[cleanup]")
{
    std::filesystem::remove_all(test_directory + "/foo");
    std::filesystem::remove_all(test_directory + "/test");
}

// TODO : Virtual Array, Pipe, Virtual Allocator tests.
