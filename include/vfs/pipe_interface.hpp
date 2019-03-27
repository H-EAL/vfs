#pragma once

#include "vfs/path.hpp"
#include "vfs/file_flags.hpp"
#include "vfs/stream_interface.hpp"


namespace vfs {

    //----------------------------------------------------------------------------------------------
    template<typename _Impl>
    class pipe_interface
        : public _Impl
    {
    public:
        //------------------------------------------------------------------------------------------
        using base_type = _Impl;
        using self_type = pipe_interface<_Impl>;
        
        //------------------------------------------------------------------------------------------
        using native_handle = typename base_type::native_handle;

    public:
        //------------------------------------------------------------------------------------------
        pipe_interface
        (
            const path              &filePath,
            file_access             access,
            file_flags              flags       = file_flags::none,
            file_attributes         attributes  = file_attributes::normal
        )
            : base_type(filePath, access, flags, attributes)
        {}

        //------------------------------------------------------------------------------------------
        pipe_interface
        (
            const path &filePath
        )
            : base_type(filePath)
        {}

    public:
        //------------------------------------------------------------------------------------------
        native_handle nativeHandle() const
        {
            return base_type::nativeHandle();
        }
		//------------------------------------------------------------------------------------------
		bool isValid()
		{
			return base_type::isValid();
		}

    public:
        //------------------------------------------------------------------------------------------
        bool waitForConnection()
        {
            return base_type::waitForConnection();
        }

        //------------------------------------------------------------------------------------------
        int64_t availableBytesToRead() const
        {
            return base_type::availableBytesToRead();
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
    };
    //----------------------------------------------------------------------------------------------

} /*vfs*/
