#pragma once

#include "vfs/path.hpp"
#include "vfs/file_flags.hpp"
#include "vfs/stream_interface.hpp"


namespace vfs {

    //----------------------------------------------------------------------------------------------
    template<typename _Impl>
    class file_interface
        : public _Impl
    {
    public:
        //------------------------------------------------------------------------------------------
        using base_type = _Impl;
        using self_type = file_interface<_Impl>;
        
        //------------------------------------------------------------------------------------------
        using native_handle = typename base_type::native_handle;

    public:
        //------------------------------------------------------------------------------------------
        file_interface
        (
            const path              &filePath,
            file_access             access,
            file_creation_options   creationOptions,
            file_flags              flags       = file_flags::none,
            file_attributes         attributes  = file_attributes::normal
        )
            : base_type(filePath, access, creationOptions, flags, attributes)
        {}

    public:
        //------------------------------------------------------------------------------------------
        static bool exists(const path &filePath)
        {
            return base_type::exists(filePath);
        }
        //------------------------------------------------------------------------------------------
        static uint64_t get_last_write_time(const path &filePath)
        {
            return base_type::get_last_write_time(filePath);
        }

    public:
        //------------------------------------------------------------------------------------------
        native_handle nativeHandle() const
        {
            return base_type::nativeHandle();
        }
        //------------------------------------------------------------------------------------------
        native_handle nativeFileMappingHandle() const
        {
            return base_type::nativeFileMappingHandle();
        }
        //------------------------------------------------------------------------------------------
        const path& fileName() const
        {
            return base_type::fileName();
        }
        //------------------------------------------------------------------------------------------
        file_access fileAccess() const
        {
            return base_type::fileAccess();
        }
        //------------------------------------------------------------------------------------------
        bool createMapping(int64_t viewSize)
        {
            return base_type::createMapping(viewSize);
		}
		//------------------------------------------------------------------------------------------
		bool isValid()
		{
			return base_type::isValid();
		}

    public:
        //------------------------------------------------------------------------------------------
        int64_t size() const
        {
            return base_type::size();
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
        bool resize(int64_t newSize)
        {
            return base_type::resize(newSize);
        }
    };
    //----------------------------------------------------------------------------------------------

} /*vfs*/
