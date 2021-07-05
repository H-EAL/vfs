
TEST_CASE("Shared memory.", "[sharedmemory]")
{
    // NOTE: For each SECTION the TEST_CASE is executed from the start.

    const auto sharedMemoryName = "mrMemory";
    auto spSharedMemory = vfs::create_shared_memory(sharedMemoryName, text.size());

    // Write.
    memcpy(spSharedMemory->cursor(), text.data(), text.size());

    SECTION("we can create shared memory")
    {
        REQUIRE(spSharedMemory->isValid());

        SECTION("we can read and write to it")
        {
            // Read.
            std::vector<uint8_t> textRead;
            textRead.resize(text.size());
            memcpy(textRead.data(), spSharedMemory->cursor(), text.size());

            REQUIRE(memcmp(textRead.data(), text.data(), text.size()) == 0);
        }
    }

    SECTION("we can open an existing shared memory object")
    {
        auto spOpenedMemory = vfs::open_shared_memory(sharedMemoryName);
        // Interestingly you are opening memory, but it's not mapped to the exact same place. i.e. spOpenedMemory->cursor() != spSharedMemory->cursor()
        auto isValid = spOpenedMemory->isValid();
        REQUIRE(isValid);

        SECTION("we can read from this existing shared memory")
        {
            std::vector<uint8_t> textRead;
            textRead.resize(text.size());
            memcpy(textRead.data(), spOpenedMemory->cursor(), text.size());

            REQUIRE(memcmp(textRead.data(), text.data(), text.size()) == 0);
        }

        SECTION("we can write to this existing shared memory")
        {
            vfs_check(text.size() == text2.size());
            memcpy(spOpenedMemory->cursor(), text2.data(), text2.size());

            std::vector<uint8_t> textRead;
            textRead.resize(text2.size());
            memcpy(textRead.data(), spSharedMemory->cursor(), text2.size());

            REQUIRE(memcmp(textRead.data(), text2.data(), text2.size()) == 0);
        }
    }

    SECTION("we cannot open a non-existing shared memory object")
    {
        auto spOpenedMemory = vfs::open_shared_memory("/mrsMemory", text.size());
        REQUIRE(!spOpenedMemory->isValid());
    }
}
