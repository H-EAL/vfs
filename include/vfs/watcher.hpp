#pragma once

#include "vfs/platform.hpp"

// File interface
#include "vfs/watcher_interface.hpp"
// Platform specific implementations
#if VFS_PLATFORM_WIN
#	include "vfs/win_watcher.hpp"
#else
#	error No directory implementation defined for the current platform
#endif


namespace vfs {

    //----------------------------------------------------------------------------------------------
    using watcher = watcher_interface<watcher_impl>;

} /*vfs*/
