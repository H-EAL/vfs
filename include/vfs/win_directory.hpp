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
        const path& dirName() const
        {
            return dirPath_;
        }

    protected:
        //------------------------------------------------------------------------------------------
        win_directory(const path &name)
            : dirPath_(name)
        {}

        //------------------------------------------------------------------------------------------
        ~win_directory()
        {}

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
                vfs_errorf("CreateDirectory(%ws) returned error: %s", dirPath.c_str(), get_last_error_as_string(errorCode).c_str());
                return false;
            }
            return true;
        }

    protected:
        //------------------------------------------------------------------------------------------
        template<typename _Dir>
        void scan(std::vector<_Dir> &subDirectories, std::vector<path> &files, int32_t recurseToDepth = 0)
        {
            auto findData = WIN32_FIND_DATA{};
            auto hFile = FindFirstFile(combine(dirPath_, L"*").c_str(), &findData);

            do
            {
                if (findData.cFileName == path(L".") || findData.cFileName == path(L".."))
                {
                    continue;
                }

                const auto &currentFilePath = combine(dirPath_, findData.cFileName);

                if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                    subDirectories.emplace_back(std::move(currentFilePath));
                }
                else
                {
                    files.emplace_back(std::move(currentFilePath));
                }

            } while (FindNextFile(hFile, &findData) != 0);
        }

    private:
        //------------------------------------------------------------------------------------------
        path dirPath_;
    };
    //----------------------------------------------------------------------------------------------

} /*vfs*/
