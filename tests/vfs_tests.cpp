#include <iostream>

#include "vfs.hpp"


int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        return EXIT_FAILURE;
    }

    const auto &filePath = vfs::path(argv[1]);

    if (vfs::directory::exists(filePath))
    {
        auto p = vfs::path::combine("foo", "bar", "bah", L"oho");
        if (vfs::create_path(p))
        {
            {
                vfs::open_read_only(vfs::path(".\\foo\\test0.txt"), vfs::file_creation_options::create_or_overwrite);
                vfs::open_read_only(vfs::path(".\\foo\\test1.txt"), vfs::file_creation_options::create_or_overwrite);
                vfs::open_read_only(vfs::path(".\\foo\\bar\\test2.txt"), vfs::file_creation_options::create_or_overwrite);
                vfs::open_read_only(vfs::path(".\\foo\\bar\\test3.txt"), vfs::file_creation_options::create_or_overwrite);
                vfs::open_read_only(vfs::path(".\\foo\\bar\\bah\\test4.txt"), vfs::file_creation_options::create_or_overwrite);
                vfs::open_read_only(vfs::path(".\\foo\\bar\\bah\\test5.txt"), vfs::file_creation_options::create_or_overwrite);
                vfs::open_read_only(vfs::path(".\\foo\\bar\\bah\\oho\\test6.txt"), vfs::file_creation_options::create_or_overwrite);
                vfs::open_read_only(vfs::path(".\\foo\\bar\\bah\\oho\\test7.txt"), vfs::file_creation_options::create_or_overwrite);
            }

            const auto pSrc = vfs::path(".\\foo");
            const auto pDst = vfs::path(".\\foo_copy");
            vfs::move_directory(pSrc, pDst);
        }
        auto dir = vfs::directory(filePath);
        dir.scan();
        auto watcher = vfs::watcher(filePath, [](const vfs::path &newDir)
        {
            std::cout << vfs::wstring_to_string(newDir) << std::endl;
        });
        watcher.startWatching(true, true);
        std::this_thread::sleep_for(std::chrono::seconds(30));
        watcher.stopWatching();
        watcher.wait();
    }
    else if (vfs::file::exists(filePath))
    {
        const auto &fv = vfs::open_read_only_view(filePath, vfs::file_creation_options::open_if_existing);
        std::cout.write(fv->cursor<const char>(), fv->totalSize());
    }
    return EXIT_SUCCESS;
}