
TEST_CASE("File.", "[file]")
{
    SECTION("we can open/create files")
    {
        SECTION("readonly")
        {
            vfs::create_path(test_directory + "\\test\\file\\readonly");

            GIVEN("an existing file")
            {
                // Read only 
                {
                    std::ofstream outfile(test_directory + "/test/file/readonly/test0.txt");
                    outfile.close();
                }

                WHEN("we open a file read_only with vfs::file::open_if_existing")
                {
                    auto spFile = vfs::open_read_only(vfs::path(test_directory + "\\test\\file\\readonly\\test0.txt"), vfs::file_creation_options::open_if_existing);
                    THEN("we succeed")
                    {
                        REQUIRE(spFile != nullptr);
                        REQUIRE(spFile->isValid());
                    }
                }

                WHEN("we open a file read_only with vfs::file::create_or_overwrite")
                {
                    auto spFile = vfs::open_read_only(vfs::path(test_directory + "\\test\\file\\readonly\\test0.txt"), vfs::file_creation_options::create_or_overwrite);
                    THEN("we succeed")
                    {
                        REQUIRE(spFile != nullptr);
                        REQUIRE(spFile->isValid());
                    }
                }

                WHEN("we open a file read_only with vfs::file::open_or_create")
                {
                    auto spFile = vfs::open_read_only(vfs::path(test_directory + "\\test\\file\\readonly\\test0.txt"), vfs::file_creation_options::open_or_create);
                    THEN("we succeed")
                    {
                        REQUIRE(spFile != nullptr);
                        REQUIRE(spFile->isValid());
                    }
                }

                WHEN("we open a file read_only with vfs::file::create_if_nonexisting")
                {
                    auto spFile = vfs::open_read_only(vfs::path(test_directory + "\\test\\file\\readonly\\test0.txt"), vfs::file_creation_options::create_if_nonexisting);
                    THEN("we fail")
                    {
                        // WE fail because we are trying to create a file that already exists.
                        REQUIRE(spFile != nullptr);
                        REQUIRE(!spFile->isValid());
                    }
                }
            }

            GIVEN("a non-existing file")
            {
                WHEN("we open a file read_only with vfs::file::open_if_existing")
                {
                    auto spFile = vfs::open_read_only(vfs::path(test_directory + "\\test\\file\\readonly\\test1.txt"), vfs::file_creation_options::open_if_existing);
                    THEN("we fail")
                    {
                        REQUIRE(spFile != nullptr);
                        REQUIRE(!spFile->isValid());
                        REQUIRE(!vfs::file::exists(test_directory + "\\test\\file\\readonly\\test1.txt"));
                    }
                }

                WHEN("we open a file read_only with vfs::file::create_or_overwrite")
                {
                    auto spFile = vfs::open_read_only(vfs::path(test_directory + "\\test\\file\\readonly\\test1.txt"), vfs::file_creation_options::create_or_overwrite);
                    THEN("we succeed")
                    {
                        REQUIRE(spFile != nullptr);
                        REQUIRE(spFile->isValid());
                        REQUIRE(vfs::file::exists(test_directory + "\\test\\file\\readonly\\test1.txt"));
                    }
                }

                WHEN("we open a file read_only with vfs::file::open_or_create")
                {
                    auto spFile = vfs::open_read_only(vfs::path(test_directory + "\\test\\file\\readonly\\test2.txt"), vfs::file_creation_options::open_or_create);
                    THEN("we succeed")
                    {
                        REQUIRE(spFile != nullptr);
                        REQUIRE(spFile->isValid());
                        REQUIRE(vfs::file::exists(test_directory + "\\test\\file\\readonly\\test2.txt"));
                    }
                }

                WHEN("we open a file read_only with vfs::file::create_if_nonexisting")
                {
                    auto spFile = vfs::open_read_only(vfs::path(test_directory + "\\test\\file\\readonly\\test3.txt"), vfs::file_creation_options::create_if_nonexisting);
                    THEN("we succeed")
                    {
                        REQUIRE(spFile != nullptr);
                        REQUIRE(spFile->isValid());
                        REQUIRE(vfs::file::exists(test_directory + "\\test\\file\\readonly\\test3.txt"));
                    }
                }
            }
        }

        SECTION("write only")
        {
            vfs::create_path(test_directory + "\\test\\file\\writeonly");

            GIVEN("an existing file")
            {
                // Write only 
                {
                    std::ofstream outfile(test_directory + "/test/file/writeonly/test0.txt");
                    outfile.close();
                }

                WHEN("we open a file write_only with vfs::file::open_if_existing")
                {
                    auto spFile = vfs::open_write_only(vfs::path(test_directory + "\\test\\file\\writeonly\\test0.txt"), vfs::file_creation_options::open_if_existing);
                    THEN("we succeed")
                    {
                        REQUIRE(spFile != nullptr);
                        REQUIRE(spFile->isValid());
                    }
                }

                WHEN("we open a file write_only with vfs::file::create_or_overwrite")
                {
                    auto spFile = vfs::open_write_only(vfs::path(test_directory + "\\test\\file\\writeonly\\test0.txt"), vfs::file_creation_options::create_or_overwrite);
                    THEN("we succeed")
                    {
                        REQUIRE(spFile != nullptr);
                        REQUIRE(spFile->isValid());
                    }
                }

                WHEN("we open a file write_only with vfs::file::open_or_create")
                {
                    auto spFile = vfs::open_write_only(vfs::path(test_directory + "\\test\\file\\writeonly\\test0.txt"), vfs::file_creation_options::open_or_create);
                    THEN("we succeed")
                    {
                        REQUIRE(spFile != nullptr);
                        REQUIRE(spFile->isValid());
                    }
                }

                WHEN("we open a file write_only with vfs::file::create_if_nonexisting")
                {
                    auto spFile = vfs::open_write_only(vfs::path(test_directory + "\\test\\file\\writeonly\\test0.txt"), vfs::file_creation_options::create_if_nonexisting);
                    THEN("we fail")
                    {
                        // WE fail because we are trying to create a file that already exists.
                        REQUIRE(spFile != nullptr);
                        REQUIRE(!spFile->isValid());
                    }
                }
            }

            GIVEN("a non-existing file")
            {
                WHEN("we open a file write_only with vfs::file::open_if_existing")
                {
                    auto spFile = vfs::open_write_only(vfs::path(test_directory + "\\test\\file\\writeonly\\test1.txt"), vfs::file_creation_options::open_if_existing);
                    THEN("we fail")
                    {
                        REQUIRE(spFile != nullptr);
                        REQUIRE(!spFile->isValid());
                        REQUIRE(!vfs::file::exists(test_directory + "\\test\\file\\writeonly\\test1.txt"));
                    }
                }

                WHEN("we open a file write_only with vfs::file::create_or_overwrite")
                {
                    auto spFile = vfs::open_write_only(vfs::path(test_directory + "\\test\\file\\writeonly\\test1.txt"), vfs::file_creation_options::create_or_overwrite);
                    THEN("we succeed")
                    {
                        REQUIRE(spFile != nullptr);
                        REQUIRE(spFile->isValid());
                        REQUIRE(vfs::file::exists(test_directory + "\\test\\file\\writeonly\\test1.txt"));
                    }
                }

                WHEN("we open a file write_only with vfs::file::open_or_create")
                {
                    auto spFile = vfs::open_write_only(vfs::path(test_directory + "\\test\\file\\writeonly\\test2.txt"), vfs::file_creation_options::open_or_create);
                    THEN("we succeed")
                    {
                        REQUIRE(spFile != nullptr);
                        REQUIRE(spFile->isValid());
                        REQUIRE(vfs::file::exists(test_directory + "\\test\\file\\writeonly\\test2.txt"));
                    }
                }

                WHEN("we open a file write_only with vfs::file::create_if_nonexisting")
                {
                    auto spFile = vfs::open_write_only(vfs::path(test_directory + "\\test\\file\\writeonly\\test3.txt"), vfs::file_creation_options::create_if_nonexisting);
                    THEN("we succeed")
                    {
                        REQUIRE(spFile != nullptr);
                        REQUIRE(spFile->isValid());
                        REQUIRE(vfs::file::exists(test_directory + "\\test\\file\\writeonly\\test3.txt"));
                    }
                }
            }
        }

        SECTION("read write")
        {
            vfs::create_path(test_directory + "\\test\\file\\readwrite");

            GIVEN("an existing file")
            {
                // Read write 
                {
                    std::ofstream outfile(test_directory + "/test/file/readwrite/test0.txt");
                    outfile.close();
                }

                WHEN("we open a file read_write with vfs::file::open_if_existing")
                {
                    auto spFile = vfs::open_read_write(vfs::path(test_directory + "\\test\\file\\readwrite\\test0.txt"), vfs::file_creation_options::open_if_existing);
                    THEN("we succeed")
                    {
                        REQUIRE(spFile != nullptr);
                        REQUIRE(spFile->isValid());
                    }
                }

                WHEN("we open a file read_write with vfs::file::create_or_overwrite")
                {
                    auto spFile = vfs::open_read_write(vfs::path(test_directory + "\\test\\file\\readwrite\\test0.txt"), vfs::file_creation_options::create_or_overwrite);
                    THEN("we succeed")
                    {
                        REQUIRE(spFile != nullptr);
                        REQUIRE(spFile->isValid());
                    }
                }

                WHEN("we open a file read_write with vfs::file::open_or_create")
                {
                    auto spFile = vfs::open_read_write(vfs::path(test_directory + "\\test\\file\\readwrite\\test0.txt"), vfs::file_creation_options::open_or_create);
                    THEN("we succeed")
                    {
                        REQUIRE(spFile != nullptr);
                        REQUIRE(spFile->isValid());
                    }
                }

                WHEN("we open a file read_write with vfs::file::create_if_nonexisting")
                {
                    auto spFile = vfs::open_read_write(vfs::path(test_directory + "\\test\\file\\readwrite\\test0.txt"), vfs::file_creation_options::create_if_nonexisting);
                    THEN("we fail")
                    {
                        // WE fail because we are trying to create a file that already exists.
                        REQUIRE(spFile != nullptr);
                        REQUIRE(!spFile->isValid());
                    }
                }
            }

            GIVEN("a non-existing file")
            {
                WHEN("we open a file read_write with vfs::file::open_if_existing")
                {
                    auto spFile = vfs::open_read_write(vfs::path(test_directory + "\\test\\file\\readwrite\\test1.txt"), vfs::file_creation_options::open_if_existing);
                    THEN("we fail")
                    {
                        REQUIRE(spFile != nullptr);
                        REQUIRE(!spFile->isValid());
                        REQUIRE(!vfs::file::exists(test_directory + "\\test\\file\\readwrite\\test1.txt"));
                    }
                }

                WHEN("we open a file read_write with vfs::file::create_or_overwrite")
                {
                    auto spFile = vfs::open_read_write(vfs::path(test_directory + "\\test\\file\\readwrite\\test1.txt"), vfs::file_creation_options::create_or_overwrite);
                    THEN("we succeed")
                    {
                        REQUIRE(spFile != nullptr);
                        REQUIRE(spFile->isValid());
                        REQUIRE(vfs::file::exists(test_directory + "\\test\\file\\readwrite\\test1.txt"));
                    }
                }

                WHEN("we open a file read_write with vfs::file::open_or_create")
                {
                    auto spFile = vfs::open_read_write(vfs::path(test_directory + "\\test\\file\\readwrite\\test2.txt"), vfs::file_creation_options::open_or_create);
                    THEN("we succeed")
                    {
                        REQUIRE(spFile != nullptr);
                        REQUIRE(spFile->isValid());
                        REQUIRE(vfs::file::exists(test_directory + "\\test\\file\\readwrite\\test2.txt"));
                    }
                }

                WHEN("we open a file read_write with vfs::file::create_if_nonexisting")
                {
                    auto spFile = vfs::open_read_write(vfs::path(test_directory + "\\test\\file\\readwrite\\test3.txt"), vfs::file_creation_options::create_if_nonexisting);
                    THEN("we succeed")
                    {
                        REQUIRE(spFile != nullptr);
                        REQUIRE(spFile->isValid());
                        REQUIRE(vfs::file::exists(test_directory + "\\test\\file\\readwrite\\test3.txt"));
                    }
                }
            }
        }
    }

    SECTION("file creation options work as intended")
    {
        GIVEN("an existing file")
        {
            WHEN("we open it with file_creation_options::open_if_existing")
            {
                auto spFile = vfs::open_read_write(vfs::path(test_directory + "\\test\\file\\readwrite\\test0.txt"), vfs::file_creation_options::open_if_existing);

                THEN("we succeed")
                {
                    REQUIRE(spFile->isValid());
                    spFile->write(text);
                    REQUIRE(spFile->size() == text.size());
                }
            }
            WHEN("we open it with file_creation_options::create_or_overwrite")
            {
                auto spFile = vfs::open_read_write(vfs::path(test_directory + "\\test\\file\\readwrite\\test0.txt"), vfs::file_creation_options::create_or_overwrite);

                THEN("we succeed and the file is overwritten")
                {
                    REQUIRE(spFile->isValid());
                    REQUIRE(spFile->size() == 0);
                }
            }
            WHEN("we open it with file_creation_options::create_if_nonexisting")
            {
                auto spFile = vfs::open_read_write(vfs::path(test_directory + "\\test\\file\\readwrite\\test0.txt"), vfs::file_creation_options::create_if_nonexisting);

                THEN("we fail")
                {
                    REQUIRE(!spFile->isValid());
                }
            }
        }

        GIVEN("a nonexisting file we want to create")
        {
            WHEN("we open it with file_creation_options::open_if_existing")
            {
                auto spFile = vfs::open_read_write(vfs::path(test_directory + "\\test\\file\\readwrite\\test5.txt"), vfs::file_creation_options::open_if_existing);

                THEN("we fail")
                {
                    REQUIRE(!spFile->isValid());
                }
            }
            WHEN("we open it with file_creation_options::create_or_overwrite")
            {
                auto spFile = vfs::open_read_write(vfs::path(test_directory + "\\test\\file\\readwrite\\test6.txt"), vfs::file_creation_options::create_or_overwrite);

                THEN("we succeed")
                {
                    REQUIRE(spFile->isValid());
                }
            }
            WHEN("we open it with file_creation_options::create_if_nonexisting")
            {
                auto spFile = vfs::open_read_write(vfs::path(test_directory + "\\test\\file\\readwrite\\test7.txt"), vfs::file_creation_options::create_if_nonexisting);

                THEN("we succeed")
                {
                    REQUIRE(spFile->isValid());
                }
            }
        }
    }

    SECTION("read write permissions of files work as intended")
    {
        GIVEN("a write only file")
        {
            auto spFile = vfs::open_write_only(vfs::path(test_directory + "\\test\\file\\readwrite\\test0.txt"), vfs::file_creation_options::open_if_existing);

            WHEN("we write into it")
            {
                auto bytesWritten = spFile->write(text);
                THEN("we succeed")
                {
                    REQUIRE(bytesWritten == text.size());
                }
            }

            WHEN("we read from it")
            {
                auto v = std::vector<uint8_t>{};
                v.resize(text.size());
                auto bytesRead = spFile->read(v);

                THEN("we fail")
                {
                    REQUIRE(bytesRead == 0);
                }
            }
        }

        GIVEN("a read only file")
        {
            auto spFile = vfs::open_read_only(vfs::path(test_directory + "\\test\\file\\readwrite\\test0.txt"), vfs::file_creation_options::open_if_existing);

            WHEN("we write into it")
            {
                auto bytesWritten = spFile->write(text) + spFile->write(text);
                THEN("we fail")
                {
                    REQUIRE(bytesWritten == 0);
                }
            }

            WHEN("we read from it")
            {
                auto v = std::vector<uint8_t>{};
                v.resize(text.size());
                auto bytesRead = spFile->read(v);

                THEN("we succeed")
                {
                    REQUIRE(bytesRead == text.size());
                }
            }
        }

        GIVEN("a read write file")
        {
            auto spFile = vfs::open_read_write(vfs::path(test_directory + "\\test\\file\\readwrite\\test0.txt"), vfs::file_creation_options::open_if_existing);

            WHEN("we write into it")
            {
                // In posix here, it seems like the file pointer is set to the end of the file.
                auto bytesWritten = spFile->write(text) + spFile->write(text);
                THEN("we succeed")
                {
                    // In posix this fails. 1335 != 890.
                    REQUIRE(spFile->size() == text.size() * 2);
                    REQUIRE(bytesWritten == text.size() * 2);
                }
            }

            WHEN("we read from it")
            {
                auto v = std::vector<uint8_t>{};
                v.resize(text.size() * 2);
                auto bytesRead = spFile->read(v);

                THEN("we succeed")
                {
                    REQUIRE(bytesRead == text.size() * 2);
                }
            }
        }
    }
}
