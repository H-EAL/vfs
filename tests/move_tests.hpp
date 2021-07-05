
TEST_CASE("Move Directory.", "[movedirectory]")
{
    SECTION("moving files and directories")
    {
        auto src = vfs::path(".\\test\\fileview\\");
        const auto dst = vfs::path(".\\test\\moved\\");

        vfs::directory::create_directory(src.str() + "readonly\\subdir");

        WHEN("we move a directory")
        {
            vfs::move_directory(src, dst, false);

            THEN("subdirectories are moved")
            {
                REQUIRE(vfs::directory::exists(dst.str() + "readwrite"));
                REQUIRE(vfs::directory::exists(dst.str() + "readonly"));
                REQUIRE(vfs::directory::exists(dst.str() + "readonly\\subdir"));

                REQUIRE(!vfs::directory::exists(src.str() + "readwrite"));
                REQUIRE(!vfs::directory::exists(src.str() + "readonly"));
                REQUIRE(!vfs::directory::exists(src.str() + "readonly\\subdir"));
            }
            THEN("subfiles are moved")
            {
                REQUIRE(vfs::file::exists(dst.str() + "readwrite\\test0.txt"));
                REQUIRE(vfs::file::exists(dst.str() + "readwrite\\test1.txt"));
                REQUIRE(vfs::file::exists(dst.str() + "readwrite\\test2.txt"));
                REQUIRE(vfs::file::exists(dst.str() + "readwrite\\test3.txt"));

                REQUIRE(!vfs::file::exists(src.str() + "readwrite\\test0.txt"));
                REQUIRE(!vfs::file::exists(src.str() + "readwrite\\test1.txt"));
                REQUIRE(!vfs::file::exists(src.str() + "readwrite\\test2.txt"));
                REQUIRE(!vfs::file::exists(src.str() + "readwrite\\test3.txt"));

                REQUIRE(vfs::file::exists(dst.str() + "readonly\\test0.txt"));
                REQUIRE(vfs::file::exists(dst.str() + "readonly\\test1.txt"));
                REQUIRE(vfs::file::exists(dst.str() + "readonly\\test2.txt"));
                REQUIRE(vfs::file::exists(dst.str() + "readonly\\test3.txt"));

                REQUIRE(!vfs::file::exists(src.str() + "readonly\\test0.txt"));
                REQUIRE(!vfs::file::exists(src.str() + "readonly\\test1.txt"));
                REQUIRE(!vfs::file::exists(src.str() + "readonly\\test2.txt"));
                REQUIRE(!vfs::file::exists(src.str() + "readonly\\test3.txt"));
            }
        }

        // Change source path!
        src = vfs::path(".\\test\\file\\");

        WHEN("we move a directory and overwrite = false")
        {
            const auto result = vfs::move_directory(src, dst, false);
            THEN("we DON'T succeed if there are files/directories of the same name in src and dst")
            {
                REQUIRE(result == false);
            }
        }

        WHEN("we move a directory and overwrite = true")
        {
            const auto result = vfs::move_directory(src, dst, true);
            THEN("we DO succeed if there are files/directories of the same name in src and dst")
            {
                REQUIRE(result == true);
            }
            THEN("files/directories from src that didn't exist in dst are properly copied")
            {
                REQUIRE(vfs::directory::exists(dst.str() + "writeonly"));
                REQUIRE(!vfs::directory::exists(src.str() + "writeonly"));

                REQUIRE(vfs::file::exists(dst.str() + "writeonly\\test0.txt"));
                REQUIRE(!vfs::file::exists(src.str() + "writeonly\\test0.txt"));
            }
            THEN("files/directories from src that already existed in dst are properly overwritten")
            {
                auto spFile = vfs::open_read_write(vfs::path(dst.str() + "readwrite\\test0.txt"), vfs::file_creation_options::open_if_existing);
                // Testing if the file was actually overwritten. The new test0.txt should be .\\test\\file\\readwrite\\test0.txt (whose size is text.size() * 2) and not .\\test\\fileview\\readwrite\\test0.txt (whose size is text.size())
                REQUIRE(spFile->isValid());
                REQUIRE(spFile->size() == text.size() * 2);
            }
        }
    }
}
