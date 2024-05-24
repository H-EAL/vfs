#pragma once

#include <dirent.h>

#include "vfs/platform.hpp"
#include "vfs/path.hpp"


namespace vfs {

    //----------------------------------------------------------------------------------------------
    using directory_impl = class posix_directory;
    //----------------------------------------------------------------------------------------------


    //----------------------------------------------------------------------------------------------
    class posix_directory
    {
    protected:
        //------------------------------------------------------------------------------------------
        static bool exists(const path &dirPath)
        {
            struct stat st{};

            if (stat(dirPath.c_str(), &st) == -1)
            {
                return false;
            }

            return (st.st_mode & S_IFMT) == S_IFDIR;
        }

        //------------------------------------------------------------------------------------------
        static bool create_directory(const path &dirPath)
        {
            const auto errorCode = mkdir(dirPath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
            if (errorCode == -1)
            {
                vfs_errorf("mkdir(%s) returned error code: %s", dirPath.c_str(),
                           get_last_error_as_string(errno).c_str());
                return false;
            }

            return true;
        }

        //------------------------------------------------------------------------------------------
        static bool delete_directory(const path &dirPath)
        {
            if (rmdir(dirPath.c_str()) == -1)
            {
                vfs_errorf("rmdir(%s) returned error code: %s", dirPath.c_str(),
                           get_last_error_as_string(errno).c_str());
                return false;
            }

            return true;
        }

        //------------------------------------------------------------------------------------------
        template<typename _Dir>
        static void scan(const path &dirPath, std::vector<_Dir> &subDirectories, std::vector<path> &files)
        {
            DIR *pDir = opendir(dirPath.c_str());
            if (pDir == nullptr)
            {
                return;
            }

            struct dirent *pEntry = nullptr;
            while ((pEntry = readdir(pDir)) != nullptr)
            {
                const auto &currentFilePath = path::combine(dirPath, pEntry->d_name);

                if (pEntry->d_type == DT_REG)
                {
                    files.emplace_back(std::move(currentFilePath));
                }
                else if (pEntry->d_type == DT_DIR)
                {
                    const auto nameLen = strlen(pEntry->d_name);
                    if (!((nameLen == 1 && pEntry->d_name[0] == '.') ||
                          (nameLen == 2 && strncmp(pEntry->d_name, "..", 2) == 0)))
                    {
                        subDirectories.emplace_back(std::move(currentFilePath));
                    }
                }
            }

            closedir(pDir);
        }
    };
    //----------------------------------------------------------------------------------------------

} /*vfs*/
