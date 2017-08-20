#pragma once

#include "vfs/path.hpp"


namespace vfs {

    //----------------------------------------------------------------------------------------------
    template<typename _Impl>
    class directory_interface
        : _Impl
    {
    public:
        //------------------------------------------------------------------------------------------
        using base_type = _Impl;
        using self_type = directory_interface<_Impl>;

    public:
        //------------------------------------------------------------------------------------------
        directory_interface(const path &dirPath)
            : path_(dirPath)
        {}

    public:
        //------------------------------------------------------------------------------------------
        static bool exists(const path &dirPath)
        {
            return base_type::exists(dirPath);
        }
        //------------------------------------------------------------------------------------------
        static bool create_directory(const path &dirPath)
        {
            return base_type::create_directory(dirPath);
        }

    public:
        //------------------------------------------------------------------------------------------
        const path& getPath() const
        {
            return path_;
        }
        //------------------------------------------------------------------------------------------
        const auto& getSubDirectories() const
        {
            return subDirectories_;
        }
        //------------------------------------------------------------------------------------------
        const auto& getFiles() const
        {
            return files_;
        }
        
        //------------------------------------------------------------------------------------------
        void scan(int32_t recurseToDepth = 0)
        {
            base_type::scan(path_, subDirectories_, files_);

            if (recurseToDepth > 0)
            {
                for (auto &subDir : subDirectories_)
                {
                    subDir.scan(recurseToDepth - 1);
                }
            }
        }

    private:
        path                    path_;
        std::vector<self_type>  subDirectories_;
        std::vector<path>       files_;
    };
    //----------------------------------------------------------------------------------------------

} /*vfs*/
