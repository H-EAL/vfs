#pragma once

#include "vfs/path.hpp"
#include "vfs/file_flags.hpp"


namespace vfs {

    //----------------------------------------------------------------------------------------------
    template<typename _Impl>
    class file_view_interface
        : _Impl
    {
    public:
        //------------------------------------------------------------------------------------------
        using base_type = _Impl;
        using self_type = file_view_interface<_Impl>;

    public:
        //------------------------------------------------------------------------------------------
        file_view_interface(file_sptr spFile, int64_t viewSize)
            : base_type(std::move(spFile), viewSize)
        {}
        //------------------------------------------------------------------------------------------
        file_view_interface(file_sptr spFile)
            : file_view_interface(std::move(spFile), 0ull)
        {}

        //------------------------------------------------------------------------------------------
        bool isValid() const
        {
            return base_type::isValid();
        }

        //------------------------------------------------------------------------------------------
        file_sptr getFile() const
        {
            return base_type::getFile();
        }
        //------------------------------------------------------------------------------------------
        int64_t totalSize() const
        {
            return base_type::totalSize();
        }

        //------------------------------------------------------------------------------------------
        int64_t read(uint8_t *dst, int64_t sizeInBytes)
        {
            return base_type::read(dst, sizeInBytes);
        }
        //------------------------------------------------------------------------------------------
        int64_t write(const uint8_t *src, int64_t sizeInBytes)
        {
            return base_type::write(src, sizeInBytes);
        }

        //------------------------------------------------------------------------------------------
        bool skip(int64_t offsetInBytes)
        {
            return base_type::skip(offsetInBytes);
        }
        //------------------------------------------------------------------------------------------
        template<typename T = uint8_t>
        auto cursor()
        {
            return base_type::cursor<T>();
        }
        //------------------------------------------------------------------------------------------
        uint8_t* cursor()
        {
            return cursor<>();
        }
    };
    //----------------------------------------------------------------------------------------------

} /*vfs*/
