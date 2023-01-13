
TEST_CASE("Fileview.", "[fileview]")
{
    SECTION("we can open/create file views")
    {
        SECTION("read only")
        {
            vfs::create_path(test_directory + "\\test\\fileview\\readonly");

            GIVEN("an existing file")
            {
                // Read only 
                {
                    auto spFile = open_read_write(test_directory + "\\test\\fileview\\readonly\\test0.txt",
                                                  vfs::file_creation_options::create_or_overwrite);
                    spFile->resize(64 * 1024);

                    REQUIRE(vfs::file::exists(test_directory + "\\test\\fileview\\readonly\\test0.txt"));
                }

                WHEN("we open a read_only_view with vfs::file::open_if_existing")
                {
                    auto spFileView = vfs::open_read_only_view(
                            vfs::path(test_directory + "\\test\\fileview\\readonly\\test0.txt"),
                            vfs::file_creation_options::open_if_existing);
                    THEN("we should succeed")
                    {
                        REQUIRE(spFileView != nullptr);
                        REQUIRE(spFileView->isValid());
                    }
                }

                WHEN("we open a read_only_view with vfs::file::create_or_overwrite")
                {
                    auto spFileView = vfs::open_read_only_view(
                            vfs::path(test_directory + "\\test\\fileview\\readonly\\test0.txt"),
                            vfs::file_creation_options::create_or_overwrite);
                    THEN("we should fail")
                    {
                        // This is expected behaviour. It does not make sense to open a read only view on a newly created file, or an overwritten file (which has also just been created). 
                        REQUIRE(spFileView != nullptr);
                        REQUIRE(!spFileView->isValid());
                    }
                }

                WHEN("we open a read_only_view with vfs::file::open_or_create")
                {
                    auto spFileView = vfs::open_read_only_view(
                            vfs::path(test_directory + "\\test\\fileview\\readonly\\test0.txt"),
                            vfs::file_creation_options::open_or_create);
                    THEN("we should succeed")
                    {
                        REQUIRE(spFileView != nullptr);
                        REQUIRE(spFileView->isValid());
                    }
                }

                WHEN("we open a read_only_view with vfs::file::create_if_nonexisting")
                {
                    auto spFileView = vfs::open_read_only_view(
                            vfs::path(test_directory + "\\test\\fileview\\readonly\\test0.txt"),
                            vfs::file_creation_options::create_if_nonexisting);
                    THEN("we should fail")
                    {
                        // WE fail because we are trying to create a file that already exists.
                        REQUIRE(spFileView == nullptr);
                    }
                }
            }

            // Given a non-existing file, we should not be able to open a fileview to it (since it's empty).
            GIVEN("a non-existing file")
            {
                WHEN("we open a read_only_view with vfs::file::open_if_existing")
                {
                    auto spFileView = vfs::open_read_only_view(
                            vfs::path(test_directory + "\\test\\fileview\\readonly\\test1.txt"),
                            vfs::file_creation_options::open_if_existing);
                    THEN("we should fail")
                    {
                        REQUIRE(spFileView == nullptr);
                        REQUIRE(!vfs::file::exists(test_directory + "\\test\\fileview\\readonly\\test1.txt"));
                    }
                }

                WHEN("we open a read_only_view with vfs::file::create_or_overwrite")
                {
                    auto spFileView = vfs::open_read_only_view(
                            vfs::path(test_directory + "\\test\\fileview\\readonly\\test1.txt"),
                            vfs::file_creation_options::create_or_overwrite);
                    THEN("we should fail")
                    {
                        // The creation of the file should succeed but the fileview will not be valid, since the file is empty and in read only.
                        REQUIRE(spFileView != nullptr);
                        REQUIRE(!spFileView->isValid());
                        REQUIRE(vfs::file::exists(test_directory + "\\test\\fileview\\readonly\\test1.txt"));
                    }
                }

                WHEN("we open a read_only_view with vfs::file::open_or_create")
                {
                    auto spFileView = vfs::open_read_only_view(
                            vfs::path(test_directory + "\\test\\fileview\\readonly\\test2.txt"),
                            vfs::file_creation_options::open_or_create);
                    THEN("we should fail")
                    {
                        // The creation of the file should succeed but the fileview will not be valid, since the file is empty and in read only.
                        REQUIRE(spFileView != nullptr);
                        REQUIRE(!spFileView->isValid());
                        REQUIRE(vfs::file::exists(test_directory + "\\test\\fileview\\readonly\\test2.txt"));
                    }
                }

                WHEN("we open a read_only_view with vfs::file::create_if_nonexisting")
                {
                    auto spFileView = vfs::open_read_only_view(
                            vfs::path(test_directory + "\\test\\fileview\\readonly\\test3.txt"),
                            vfs::file_creation_options::create_if_nonexisting);
                    THEN("we should fail")
                    {
                        // The creation of the file should succeed but the fileview will not be valid, since the file is empty and in read only.
                        REQUIRE(spFileView != nullptr);
                        REQUIRE(!spFileView->isValid());
                        REQUIRE(vfs::file::exists(test_directory + "\\test\\fileview\\readonly\\test3.txt"));
                    }
                }
            }
        }

        SECTION("read write")
        {
            vfs::create_path(test_directory + "\\test\\fileview\\readwrite");

            // TODO: have a better error for the following cases where we are supposed to fail.
            GIVEN("an existing file")
            {
                // Read only 
                {
                    auto spFile = open_read_write(test_directory + "\\test\\fileview\\readwrite\\test0.txt", vfs::file_creation_options::create_or_overwrite);
                    spFile->resize(text.size());

                    REQUIRE(vfs::file::exists(test_directory + "\\test\\fileview\\readwrite\\test0.txt"));
                }

                WHEN("we open a read_write_view with vfs::file::open_if_existing")
                {
                    auto spFileView = vfs::open_read_write_view(vfs::path(test_directory + "\\test\\fileview\\readwrite\\test0.txt"), vfs::file_creation_options::open_if_existing);
                    THEN("we should succeed")
                    {
                        REQUIRE(spFileView != nullptr);
                        REQUIRE(spFileView->isValid());
                    }
                }

                WHEN("we open a read_write_view with vfs::file::create_or_overwrite")
                {
                    auto spFileView = vfs::open_read_write_view(vfs::path(test_directory + "\\test\\fileview\\readwrite\\test0.txt"), vfs::file_creation_options::create_or_overwrite);
                    THEN("we should fail")
                    {
                        // This is expected behaviour. It does not make sense to open a read write view on a newly created file, or an overwritten file (which has also just been created). 
                        REQUIRE(spFileView != nullptr);
                        REQUIRE(!spFileView->isValid());
                    }
                }

                WHEN("we open a read_write_view with vfs::file::open_or_create")
                {
                    auto spFileView = vfs::open_read_write_view(vfs::path(test_directory + "\\test\\fileview\\readwrite\\test0.txt"), vfs::file_creation_options::open_or_create);
                    THEN("we should succeed")
                    {
                        REQUIRE(spFileView != nullptr);
                        REQUIRE(spFileView->isValid());
                    }
                }

                WHEN("we open a read_write_view with vfs::file::create_if_nonexisting")
                {
                    auto spFileView = vfs::open_read_write_view(vfs::path(test_directory + "\\test\\fileview\\readwrite\\test0.txt"), vfs::file_creation_options::create_if_nonexisting);
                    THEN("we should fail")
                    {
                        // We can't open an existing file with create_if_nonexisting.
                        REQUIRE(spFileView == nullptr);
                    }
                }
            }

            GIVEN("a non-existing file")
            {
                auto viewSize = 50;
                WHEN("we open a read_write_view with vfs::file::open_if_existing with viewSize")
                {
                    auto spFileView = vfs::open_read_write_view(vfs::path(test_directory + "\\test\\fileview\\readwrite\\test1.txt"), vfs::file_creation_options::open_if_existing, vfs::file_flags::none, vfs::file_attributes::normal, viewSize);
                    THEN("we should fail")
                    {
                        REQUIRE(spFileView == nullptr);
                        REQUIRE(!vfs::file::exists(test_directory + "\\test\\fileview\\readwrite\\test1.txt"));
                    }
                }

                WHEN("we open a read_write_view with vfs::file::create_or_overwrite with viewSize")
                {
                    auto spFileView = vfs::open_read_write_view(vfs::path(test_directory + "\\test\\fileview\\readwrite\\test1.txt"), vfs::file_creation_options::create_or_overwrite, vfs::file_flags::none, vfs::file_attributes::normal, viewSize);
                    THEN("we should succeed")
                    {
                        REQUIRE(spFileView != nullptr);
                        REQUIRE(spFileView->isValid());
                        REQUIRE(vfs::file::exists(test_directory + "\\test\\fileview\\readwrite\\test1.txt"));
                    }
                }

                WHEN("we open a read_write_view with vfs::file::open_or_create with viewSize")
                {
                    auto spFileView = vfs::open_read_write_view(vfs::path(test_directory + "\\test\\fileview\\readwrite\\test2.txt"), vfs::file_creation_options::open_or_create, vfs::file_flags::none, vfs::file_attributes::normal, viewSize);
                    THEN("we should succeed")
                    {
                        REQUIRE(spFileView != nullptr);
                        REQUIRE(spFileView->isValid());
                        REQUIRE(vfs::file::exists(test_directory + "\\test\\fileview\\readwrite\\test2.txt"));
                    }
                }

                WHEN("we open a read_write_view with vfs::file::create_if_nonexisting with viewSize")
                {
                    auto spFileView = vfs::open_read_write_view(vfs::path(test_directory + "\\test\\fileview\\readwrite\\test3.txt"), vfs::file_creation_options::create_if_nonexisting, vfs::file_flags::none, vfs::file_attributes::normal, viewSize);
                    THEN("we should succeed")
                    {
                        REQUIRE(spFileView != nullptr);
                        REQUIRE(spFileView->isValid());
                        REQUIRE(vfs::file::exists(test_directory + "\\test\\fileview\\readwrite\\test3.txt"));
                    }
                }
            }
        }
    }

    SECTION("read write permissions of file views work as intended")
    {
        GIVEN("a read write fileview")
        {
            auto spFileView = vfs::open_read_write_view(vfs::path(test_directory + "\\test\\fileview\\readwrite\\test0.txt"), vfs::file_creation_options::open_if_existing);
            REQUIRE(spFileView != nullptr);
            REQUIRE(spFileView->isValid());

            WHEN("we write into it")
            {
                // access violation writing location.
                auto bytesWritten = spFileView->write(text);
                THEN("we should succeed")
                {
                    REQUIRE(bytesWritten == text.size());
                }
            }

            WHEN("we read from it")
            {
                auto v = std::vector<uint8_t>{};
                v.resize(text.size());
                auto bytesRead = spFileView->read(v);

                THEN("we should succeed")
                {
                    REQUIRE(bytesRead == text.size());
                }
            }
        }

        GIVEN("a read only fileview")
        {
            auto spFileView = vfs::open_read_only_view(vfs::path(test_directory + "\\test\\fileview\\readwrite\\test0.txt"), vfs::file_creation_options::open_if_existing);
            REQUIRE(spFileView != nullptr);
            REQUIRE(spFileView->isValid());

            // Cannot test the case of writing into it, since this will just lead to an OS "Access violation writing location" exception.
            WHEN("we read from it")
            {
                auto v = std::vector<uint8_t>{};
                v.resize(text.size());
                auto bytesRead = spFileView->read(v);

                THEN("we should succeed")
                {
                    REQUIRE(bytesRead == text.size());
                }
            }
        }
    }
}
