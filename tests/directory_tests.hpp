
TEST_CASE("Directory.", "[directory]")
{
    std::filesystem::remove_all(".\\foo");
    std::filesystem::remove_all(".\\test");

    SECTION("vfs::directory::exists works as intended")
    {
        std::filesystem::create_directories(".\\test\\pi\\ka\\chu");

        REQUIRE(vfs::directory::exists(".\\test"));
        REQUIRE(vfs::directory::exists(".\\test\\pi"));
        REQUIRE(vfs::directory::exists(".\\test\\pi\\ka"));
        REQUIRE(vfs::directory::exists(".\\test\\pi\\ka\\chu"));
        REQUIRE(!vfs::directory::exists(".\\test\\pi\\ka\\chu\\go"));
    }

    SECTION("we can create directories")
    {
        vfs::directory::create_directory(".\\foo");
        vfs::directory::create_directory(".\\foo\\bar");

        REQUIRE(vfs::directory::exists(".\\foo"));
        REQUIRE(vfs::directory::exists(".\\foo\\bar"));
    }

    SECTION("we can create a path")
    {
        auto path = std::string(".\\test\\I\\am\\a\\long\\path\\nice_to_meet_you");
        vfs::create_path(path);

        REQUIRE(vfs::directory::exists(path));
    }
}