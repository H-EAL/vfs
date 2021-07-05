
TEST_CASE("Directory.", "[directory]")
{
    SECTION("vfs::directory::exists works as intended")
    {
        std::filesystem::create_directories(test_directory + "/test/pi/ka/chu");

        REQUIRE(vfs::directory::exists(test_directory + "\\test"));
        REQUIRE(vfs::directory::exists(test_directory + "\\test\\pi"));
        REQUIRE(vfs::directory::exists(test_directory + "\\test\\pi\\ka"));
        REQUIRE(vfs::directory::exists(test_directory + "\\test\\pi\\ka\\chu"));
        REQUIRE(!vfs::directory::exists(test_directory + "\\test\\pi\\ka\\chu\\go"));
    }

    SECTION("we can create directories")
    {
        vfs::directory::create_directory(test_directory + "\\foo");
        vfs::directory::create_directory(test_directory + "\\foo\\bar");

        REQUIRE(vfs::directory::exists(test_directory + "\\foo"));
        REQUIRE(vfs::directory::exists(test_directory + "\\foo\\bar"));
    }

    SECTION("we can create a path")
    {
        auto path = std::string(test_directory + "\\test\\I\\am\\a\\long\\path\\nice_to_meet_you");
        vfs::create_path(path);

        REQUIRE(vfs::directory::exists(path));
    }
}
