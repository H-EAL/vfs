#pragma once

#include "vfs/platform.hpp"

// Interface
#include "vfs/virtual_allocator_interface.hpp"
// Platform specific implementations
#if VFS_PLATFORM_WIN
#	include "vfs/win_virtual_allocator.hpp"
#elif VFS_PLATFORM_POSIX
#   include "vfs/posix_virtual_allocator.hpp"
#else
#	error No virtual allocator implementation defined for the current platform
#endif


namespace vfs {

    //----------------------------------------------------------------------------------------------
    using virtual_allocator = virtual_allocator_interface<virtual_allocator_impl>;

} /*vfs*/
