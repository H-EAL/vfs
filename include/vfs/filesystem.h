#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <sys/stat.h>

#include "dirent.h"
#include "error.h"
#include "string_utils.h"


namespace vizua { namespace filesystem {

    enum class auto_scan
    {
        on  = true,
        off = false
    };

    class directory
    {
    public:
        directory(const std::string &_path, auto_scan autoScan = auto_scan::off, int32_t recurseToDepth = 0)
            : path_(_path)
        {
            if (autoScan == auto_scan::on)
            {
                scan(recurseToDepth);
            }
        }

        void scan(int32_t recurseToDepth = 0)
        {
            scoped_dir __d(path_);

            if (!__d.dirp)
            {
                vizua_errorf("Could not open directory: {}", path_);
                return;
            }

            while (dirent *pEntry = readdir(__d.dirp))
            {
                switch (pEntry->d_type)
                {
                case DT_REG:
                    files_.emplace_back(pEntry->d_name, pEntry->d_namlen);
                    break;

                case DT_DIR:
                    if (!((pEntry->d_namlen == 1 && pEntry->d_name[0] == '.') ||
                        (pEntry->d_namlen == 2 && pEntry->d_name[0] == '.' && pEntry->d_name[1] == '.')))
                    {
                        subDirectories_.emplace_back(std::string(pEntry->d_name, pEntry->d_namlen));
                    }
                    break;

                default:
                    break;
                }
            }

            if (recurseToDepth > 0)
            {
                for (auto &subDir : subDirectories_)
                {
                    subDir.scan(recurseToDepth - 1);
                }
            }
        }

        const auto& path() const
        {
            return path_;
        }
        
        const auto& subDirectories() const
        {
            return subDirectories_;
        }

        const auto &files() const
        {
            return files_;
        }

    private:
        struct scoped_dir
        {
            scoped_dir(const std::string &path)
                : dirp(opendir(path.c_str()))
            {}

            ~scoped_dir()
            {
                if (dirp)
                {
                    closedir(dirp);
                }
            }

            DIR *dirp;
        };

    private:
        const std::string           path_;
        std::vector<directory>      subDirectories_;
        std::vector<std::string>    files_;
    };



    inline auto read_file_to_string(const std::string &filePath)
    {
        std::ifstream ifs(filePath);
        std::string content((std::istreambuf_iterator<char>(ifs)),
            (std::istreambuf_iterator<char>()));

        return content;
    }

    inline auto read_file_to_vector(const std::string &filePath)
    {
        std::ifstream ifs(filePath);
        std::vector<int8_t> buffer((std::istreambuf_iterator<char>(ifs)),
            (std::istreambuf_iterator<char>()));

        return buffer;
    }

    inline int64_t get_file_size(const std::string &filePath)
    {
        struct _stat statBuf;
        const auto rc = _stat(filePath.c_str(), &statBuf);
        return rc == 0 ? statBuf.st_size : -1;
    }

    inline bool exists(const std::string &path)
    {
        struct stat info;
        return stat(path.c_str(), &info) == 0;
    }

    inline bool create_directory(const std::string &path)
    {
        if (CreateDirectoryA(path.c_str(), nullptr) == FALSE)
        {
            const auto errorCode = GetLastError();
            vizua_errorf("CreateDirectoryA({}) returned error: {}", path, get_last_error_as_string(errorCode));
            return false;
        }
        return true;
    }

    inline bool create_path(const std::string &path)
    {
        auto folders = split_string(path, "/\\");
        if (folders.empty())
        {
            return false;
        }

        size_t i = 0;
        std::string currentPath;
        if (path[0] == '\\' && path[1] == '\\')
        {
            currentPath = "\\\\" + folders[0] + "\\";
            ++i;
        }
        else if (path[1] == ':' && path[2] == '\\')
        {
            currentPath = folders[0] + "\\";
            ++i;
        }

        for (; i < folders.size(); ++i)
        {
            currentPath += folders[i] + "/";
            if (!exists(currentPath))
            {
                if (!create_directory(currentPath))
                {
                    return false;
                }
            }
        }

        return true;
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

    inline bool move(const std::string &src, const std::string &dst, bool overwrite = false, int32_t maxAttempts = 1)
    {
        auto flags = MOVEFILE_COPY_ALLOWED | MOVEFILE_WRITE_THROUGH;
        if (overwrite)
        {
            flags |= MOVEFILE_REPLACE_EXISTING;
        }

        auto attempts = 0;
        while (MoveFileExA(src.c_str(), dst.c_str(), flags) == FALSE)
        {
            if (++attempts == maxAttempts)
            {
                const auto errorCode = GetLastError();
                vizua_errorf("MoveFileExA({}, {}, MOVEFILE_COPY_ALLOWED | MOVEFILE_WRITE_THROUGH) failed after {} attempts, returned error: {}", src, dst, attempts, get_last_error_as_string(errorCode));
                return false;
            }
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
