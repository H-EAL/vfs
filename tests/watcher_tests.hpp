
TEST_CASE("Watcher.", "[watcher]")
{
    auto filePath                   = std::string(test_directory + "\\test\\watcher");
    auto filesDirectoriesWatched    = std::unordered_set<std::string>();
    auto filesDirectoriesCreated    = std::unordered_set<std::string>();

    auto numFilesToCreate       = 5;
    auto numDirectoriesToCreate = 3;

    vfs::directory::create_directory(filePath);

    // TODO. test for watching only files or folders.
    SECTION("we can watch over the files and folders of a directory")
    {
        auto watcher = vfs::watcher(filePath, [&filesDirectoriesWatched](const vfs::path &newDir)
            {
                std::error_code err{};
                if (std::filesystem::exists(newDir.str()) && !std::filesystem::is_empty(newDir.str(), err))
                {
                    // This callback could be called when the folder is empty since callback is called in watcher::run() before waiting for the event.
                    // Using directory_iterator with an empty directory cause program to crash.
                    for (const auto &entry : std::filesystem::directory_iterator(newDir.str()))
                    {
                        filesDirectoriesWatched.insert(entry.path().filename().string());
                    }
                }
            });

        watcher.startWatching(true, true);
        
        GIVEN("the creation of files and directories in a watched directory")
        {
            // Start creating files and directories.
            for (auto i = 0; i < numFilesToCreate; ++i)
            {
                auto fileName = "test" + std::to_string(i) + ".txt";
                vfs::open_read_only(vfs::path(filePath + "\\" + fileName), vfs::file_creation_options::create_or_overwrite);
                filesDirectoriesCreated.insert(fileName);

            }
            for (auto i = 0; i < numDirectoriesToCreate; ++i)
            {
                auto dirName = "dir" + std::to_string(i);
                vfs::directory::create_directory(filePath + "\\" + dirName);
                filesDirectoriesCreated.insert(dirName);
            }

            // Sleep for a bit to wait for watcher.
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(500ms);

            THEN("the watcher will watch all of them!")
            {
                auto isValid = true;
                for (auto &fileDirCreated : filesDirectoriesCreated)
                {
                    if (filesDirectoriesWatched.find(fileDirCreated) == filesDirectoriesWatched.end())
                    {
                        vfs_errorf("%s was not caught by the watcher", fileDirCreated.c_str());
                        isValid = false;
                    }
                }

                REQUIRE(isValid);
            }
        }
    }
}
