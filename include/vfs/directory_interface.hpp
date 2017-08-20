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
            : base_type(dirPath)
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
        const path& dirName() const
        {
            return base_type::dirName();
        }
        
        //------------------------------------------------------------------------------------------
        void scan(int32_t recurseToDepth = 0)
        {
            base_type::scan(subDirectories_, files_, recurseToDepth);

            if (recurseToDepth > 0)
            {
                for (auto &subDir : subDirectories_)
                {
                    subDir.scan(recurseToDepth - 1);
                }
            }
        }

    private:
        std::vector<self_type>  subDirectories_;
        std::vector<path>       files_;
    };
    //----------------------------------------------------------------------------------------------

} /*vfs*/
