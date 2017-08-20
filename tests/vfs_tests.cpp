#include <iostream>
#include <codecvt>
#include <string>

#include "vfs.hpp"


//--------------------------------------------------------------------------------------------------
inline std::string wstring_to_string(const std::wstring &toConvert)
{
    using convert_type = std::codecvt_utf8<wchar_t>;
    static std::wstring_convert<convert_type, wchar_t> converter;
    return converter.to_bytes(toConvert);
}

int wmain(int argc, wchar_t *argv[])
{
    if (argc < 2)
    {
        return EXIT_FAILURE;
    }

    const auto &filePath = vfs::path(argv[1]);

    if (vfs::directory::exists(filePath))
    {
        auto dir = vfs::directory(filePath);
        dir.scan();
        auto watcher = vfs::watcher(filePath, [](const vfs::path &newDir)
        {
            std::cout << wstring_to_string(newDir) << std::endl;
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