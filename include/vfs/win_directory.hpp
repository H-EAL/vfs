#pragma once

#include "vfs/platform.hpp"
#include "vfs/path.hpp"


namespace vfs {

    //----------------------------------------------------------------------------------------------
    using directory_impl = class win_directory;
    //----------------------------------------------------------------------------------------------


    //----------------------------------------------------------------------------------------------
    class win_directory
    {
    protected:
        //------------------------------------------------------------------------------------------
        static bool exists(const path &dirPath)
        {
            const auto dwAttrib = GetFileAttributes(dirPath.c_str());
            return (dwAttrib != INVALID_FILE_ATTRIBUTES) && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
        }

        //------------------------------------------------------------------------------------------
        static bool create_directory(const path &dirPath)
        {
            if (CreateDirectory(dirPath.c_str(), nullptr) == FALSE)
            {
                const auto errorCode = GetLastError();
                vfs_errorf("CreateDirectory(%s) returned error: %s", dirPath.c_str(), get_last_error_as_string(errorCode).c_str());
                return false;
            }
            return true;
        }

        //------------------------------------------------------------------------------------------
        static bool delete_directory(const path &dirPath)
        {
            if (RemoveDirectory(dirPath.c_str()) == FALSE)
            {
                const auto errorCode = GetLastError();
                vfs_errorf("RemoveDirectory(%s) returned error: %s", dirPath.c_str(), get_last_error_as_string(errorCode).c_str());
                return false;
            }
            return true;
        }

        //------------------------------------------------------------------------------------------
        template<typename _Dir>
        static void scan(const path &dirPath, std::vector<_Dir> &subDirectories, std::vector<path> &files)
        {
            auto findData = WIN32_FIND_DATA{};
            auto hFile = FindFirstFile(path::combine(dirPath, path("*")).c_str(), &findData);

            if (hFile == INVALID_HANDLE_VALUE)
            {
                const auto errorCode = GetLastError();
                vfs_errorf("FindFirstFile(%s) returned error: %s", dirPath.c_str(), get_last_error_as_string(errorCode).c_str());
                return;
            }

            do
            {
                if (findData.cFileName == path(".").str() || findData.cFileName == path("..").str())
                {
                    continue;
                }

                const auto &currentFilePath = path::combine(dirPath, path(findData.cFileName));

                if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                    subDirectories.emplace_back(std::move(currentFilePath));
                }
                else
                {
                    files.emplace_back(std::move(currentFilePath));
                }

            } while (FindNextFile(hFile, &findData) != 0);

            const auto errorCode = GetLastError();
            if (errorCode != ERROR_NO_MORE_FILES)
            {
                vfs_errorf("FindNextFile(%s) returned error: %s", dirPath.c_str(), get_last_error_as_string(errorCode).c_str());
            }
            
            FindClose(hFile);
        }
    };
    //----------------------------------------------------------------------------------------------

} /*vfs*/
