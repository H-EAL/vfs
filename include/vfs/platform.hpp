#pragma once

// Define every supported platform to 0 so it can be used in #if statements
#define VFS_PLATFORM_WIN	    (0)
#define VFS_PLATFORM_POSIX	    (0)

// Define only the current platform to 1
#if defined(_WIN32)
#	undef  VFS_PLATFORM_WIN
#	define VFS_PLATFORM_WIN	    (1)
#   define WIN32_LEAN_AND_MEAN
#   define VC_EXTRALEAN
#   include <windows.h>
#elif defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
#	undef  VFS_PLATFORM_POSIX
#	define VFS_PLATFORM_POSIX	(1)
#endif

// Cache line size to avoid false sharing
#define VFS_CACHE_LINE_SIZE    (64)


#include "vfs/logging.hpp"
