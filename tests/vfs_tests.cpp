#include <iostream>

#include "vfs.hpp"


int wmain(int argc, wchar_t *argv[])
{
    if (argc < 2)
    {
        return EXIT_FAILURE;
    }

    const auto &filePath = vfs::path(argv[1]);

    if (vfs::directory::exists(filePath))
    {
        auto p = vfs::path::combine(vfs::path(".\\foo"), vfs::path("bar"), std::string("bah"), std::wstring(L"bdsaah"));
        vfs::create_path(p);
        auto dir = vfs::directory(filePath);
        dir.scan();
        auto watcher = vfs::watcher(filePath, [](const vfs::path &newDir)
        {
            std::cout << vfs::wstring_to_string(newDir) << std::endl;
        });
        watcher.startWatching(true, true);
        Sleep(60000);
        watcher.stopWatching();
        watcher.wait();
    }
    else if (vfs::file::exists(filePath))
    {
        const auto &fv = vfs::open_read_only_view(filePath, vfs::file_creation_options::open_if_existant);
        std::cout.write(fv->cursor<const char>(), fv->totalSize());
    }
    return EXIT_SUCCESS;
}