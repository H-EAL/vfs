#pragma once

#include <memory>
#include "vfs/platform.hpp"

// Pipe interface
#include "vfs/pipe_interface.hpp"
// Platform specific implementations
#if VFS_PLATFORM_WIN
#	include "vfs/win_pipe.hpp"
#elif VFS_PLATFORM_POSIX
#   include "vfs/posix_pipe.hpp"
#else
#	error No pipe implementation defined for the current platform
#endif

#include "vfs/path.hpp"
#include "vfs/file_flags.hpp"
#include "vfs/stream_interface.hpp"


namespace vfs {

    //----------------------------------------------------------------------------------------------
    using pipe          = pipe_interface<pipe_impl>;
    using pipe_stream   = stream_interface<pipe>;
    //----------------------------------------------------------------------------------------------
    using pipe_sptr     = std::shared_ptr<pipe_stream>;
    using pipe_wptr     = std::weak_ptr<pipe_stream>;
    //----------------------------------------------------------------------------------------------
    
    //----------------------------------------------------------------------------------------------
    inline auto connect_to_named_pipe
    (
        const path              &pipeName,
        file_access             fileAccess,
        file_flags              fileFlags = file_flags::none,
        file_attributes         fileAttributes = file_attributes::normal
    )
    {
        return pipe_sptr(new pipe_stream(pipeName, fileAccess, fileFlags, fileAttributes));
    }
    //----------------------------------------------------------------------------------------------


    //----------------------------------------------------------------------------------------------
    inline auto create_named_pipe
    (
        const path              &pipeName,
        pipe_access             pipeAccess
    )
    {
        return pipe_sptr(new pipe_stream(pipeName, pipeAccess));
    }
    //----------------------------------------------------------------------------------------------

} /*vfs*/
