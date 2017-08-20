#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <sys/stat.h>

#include "dirent.h"
#include "error.h"
#include "string_utils.h"


namespace vizua { namespace filesystem {

    inline int64_t get_file_size(const std::string &filePath)
    {
        struct _stat statBuf;
        const auto rc = _stat(filePath.c_str(), &statBuf);
        return rc == 0 ? statBuf.st_size : -1;
    }


    inline bool delete_file(const std::string &path)
    {
        if (DeleteFileA(path.c_str()) == FALSE)
        {
            const auto errorCode = GetLastError();
            vizua_errorf("DeleteFileA({}) returned error: {}", path, get_last_error_as_string(errorCode));
            return false;
        }
        return true;
    }

    inline bool delete_directory(const std::string &path)
    {
        auto dir = directory(path, auto_scan::on);
        for (const auto &subDir : dir.subDirectories())
        {
            delete_directory(path + subDir.path());
        }
        for (const auto &file : dir.files())
        {
            delete_file(path + file);
        }

        if (RemoveDirectoryA(path.c_str()) == FALSE)
        {
            const auto errorCode = GetLastError();
            vizua_errorf("RemoveDirectoryA({}) returned error: {}", path, get_last_error_as_string(errorCode));
            return false;
        }

        return true;
    }

    //----------------------------------------------------------------------------------------------
    std::vector<std::string> find_files_in_folder_by_extension(const std::string &folder, const std::string &extension)
    {
        auto files = std::vector<std::string>{};

        const auto dir = filesystem::directory(folder, filesystem::auto_scan::on);
        for (const auto &fileName : dir.files())
        {
            const auto &ext = get_extension(fileName);
            if (ext == extension)
            {
                files.emplace_back(dir.path() + "/" + fileName);
            }
        }

        return files;
    }

    //----------------------------------------------------------------------------------------------
    std::string find_file_in_folder_by_extension(const std::string &folder, const std::string &extension)
    {
        const auto &files = find_files_in_folder_by_extension(folder, extension);

        if (files.size() > 1)
        {
            vizua_warningf("Found more than 1 .{} file in {}. Using the first one: {}", extension, folder, files[0]);
        }

        return files.empty() ? "" : files[0];
    }

} /*filesystem*/ } /*vizua*/
