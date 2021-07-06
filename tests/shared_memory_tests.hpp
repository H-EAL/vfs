
TEST_CASE("Shared memory.", "[sharedmemory]")
{
    // NOTE: For each SECTION the TEST_CASE is executed from the start.
    SECTION("we can create shared memory")
    {
        // In posix, shared memory must be prefaced by a slash, but in windows a slash in the name is invalid.
#if VFS_PLATFORM_WIN
        const auto sharedMemoryName = "mrsMemory";
#elif VFS_PLATFORM_POSIX
        const auto sharedMemoryName = "/mrsMemory";
#endif
        auto spSharedMemory = vfs::create_shared_memory(sharedMemoryName, text.size());
        REQUIRE(spSharedMemory->isValid());

        SECTION("we can read and write to shared memory")
        {
            // Write.
            memcpy(spSharedMemory->cursor(), text.data(), text.size());

            // Read.
            std::vector<uint8_t> textRead;
            textRead.resize(text.size());
            memcpy(textRead.data(), spSharedMemory->cursor(), text.size());

            REQUIRE(memcmp(textRead.data(), text.data(), text.size()) == 0);
        }

        SECTION("we can open an existing shared memory object")
        {
            // Write to the existing shared memory.
            memcpy(spSharedMemory->cursor(), text.data(), text.size());

            auto spOpenedMemory = vfs::open_shared_memory(sharedMemoryName);
            // Interestingly you are opening memory, but it's not mapped to the exact same place. i.e. spOpenedMemory->cursor() != spSharedMemory->cursor()
            REQUIRE(spOpenedMemory->isValid());

            SECTION("we can read from this existing shared memory")
            {
                std::vector<uint8_t> textRead;
                textRead.resize(text.size());
                memcpy(textRead.data(), spOpenedMemory->cursor(), text.size());

                REQUIRE(memcmp(textRead.data(), text.data(), text.size()) == 0);
            }

            SECTION("we can write to this existing shared memory")
            {
                REQUIRE(text.size() == text2.size());
                memcpy(spOpenedMemory->cursor(), text2.data(), text2.size());

                std::vector<uint8_t> textRead;
                textRead.resize(text2.size());
                memcpy(textRead.data(), spSharedMemory->cursor(), text2.size());

                REQUIRE(memcmp(textRead.data(), text2.data(), text2.size()) == 0);
            }
        }

        SECTION("we cannot open a non-existing shared memory object")
        {
#if VFS_PLATFORM_WIN
            const auto openSharedMemoryName = "mrMemory";
#elif VFS_PLATFORM_POSIX
            const auto openSharedMemoryName = "/mrMemory";
#endif
            auto spOpenedMemory = vfs::open_shared_memory(openSharedMemoryName, text.size());
            REQUIRE(!spOpenedMemory->isValid());
        }
    }
}
