#pragma once

#include "vfs/platform.hpp"

// File interface
#include "vfs/directory_interface.hpp"
// Platform specific implementations
#if VFS_PLATFORM_WIN
#	include "vfs/win_directory.hpp"
#else
#	error No directory implementation defined for the current platform
#endif


namespace vfs {

    //----------------------------------------------------------------------------------------------
    using directory = directory_interface<directory_impl>;

} /*vfs*/
