#pragma once

#include "vfs/platform.hpp"

// File interface
#include "vfs/file_view_interface.hpp"
// Platform specific implementations
#if VFS_PLATFORM_WIN
#	include "vfs/win_file_view.hpp"
#else
#	error No file_view implementation defined for the current platform
#endif

#include "vfs/file.hpp"

namespace vfs {

    //----------------------------------------------------------------------------------------------
    using file_view         = file_view_interface<file_view_impl>;
    using file_view_stream  = stream_interface<file_view>;
    //----------------------------------------------------------------------------------------------
    using file_view_sptr    = std::shared_ptr<file_view_stream>;
    using file_view_wptr    = std::weak_ptr<file_view_stream>;
    //----------------------------------------------------------------------------------------------

    //----------------------------------------------------------------------------------------------
    inline auto open_read_only_view
    (
        const path              &fileName,
        file_creation_options   creationOptions,
        file_flags              fileFlags = file_flags::none,
        file_attributes         fileAttributes = file_attributes::normal
    )
    {
        auto spFile = open_read_only(fileName, creationOptions, fileFlags, fileAttributes);
        return file_view_sptr(new file_view_stream(std::move(spFile)));
    }
    //----------------------------------------------------------------------------------------------

    //----------------------------------------------------------------------------------------------
    inline auto open_read_write_view
    (
        const path              &fileName,
        file_creation_options   creationOptions,
        file_flags              fileFlags = file_flags::none,
        file_attributes         fileAttributes = file_attributes::normal
    )
    {
        auto spFile = open_read_write(fileName, creationOptions, fileFlags, fileAttributes);
        return file_view_sptr(new file_view_stream(std::move(spFile)));
    }
    //----------------------------------------------------------------------------------------------

} /*vfs*/
