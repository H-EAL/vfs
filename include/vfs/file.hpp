#pragma once

#include <memory>
#include "vfs/platform.hpp"

// File interface
#include "vfs/file_interface.hpp"
// Platform specific implementations
#if VFS_PLATFORM_WIN
#	include "vfs/win_file.hpp"
#elif VFS_PLATFORM_POSIX
#   include "vfs/posix_file.hpp"
#else
#	error No file implementation defined for the current platform
#endif

#include "vfs/path.hpp"
#include "vfs/file_flags.hpp"
#include "vfs/stream_interface.hpp"


namespace vfs {

    //----------------------------------------------------------------------------------------------
    using file          = file_interface<file_impl>;
    using file_stream   = stream_interface<file>;
    //----------------------------------------------------------------------------------------------
    using file_sptr     = std::shared_ptr<file_stream>;
    using file_wptr     = std::weak_ptr<file_stream>;
    //----------------------------------------------------------------------------------------------

    //----------------------------------------------------------------------------------------------
    inline auto open_read_only
    (
        const path              &fileName,
        file_creation_options   creationOptions,
        file_flags              fileFlags = file_flags::none,
        file_attributes         fileAttributes = file_attributes::normal
    )
    {
        return file_sptr(new file_stream(fileName, file_access::read_only, creationOptions, fileFlags, fileAttributes));
    }
    //----------------------------------------------------------------------------------------------


    //----------------------------------------------------------------------------------------------
    inline auto open_write_only
    (
        const path              &fileName,
        file_creation_options   creationOptions,
        file_flags              fileFlags = file_flags::none,
        file_attributes         fileAttributes = file_attributes::normal
    )
    {
        return file_sptr(new file_stream(fileName, file_access::write_only, creationOptions, fileFlags, fileAttributes));
    }
    //----------------------------------------------------------------------------------------------


    //----------------------------------------------------------------------------------------------
    inline auto open_read_write
    (
        const path              &fileName,
        file_creation_options   creationOptions,
        file_flags              fileFlags = file_flags::none,
        file_attributes         fileAttributes = file_attributes::normal
    )
    {
        return file_sptr(new file_stream(fileName, file_access::read_write, creationOptions, fileFlags, fileAttributes));
    }
    //----------------------------------------------------------------------------------------------

} /*vfs*/
