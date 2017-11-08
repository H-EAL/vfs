#pragma once

// Define every supported platform to 0 so it can be used in #if statements
#define VFS_PLATFORM_WIN	    (0)
#define VFS_PLATFORM_POSIX	    (0)

// Define only the current platform to 1
#if defined(_WIN32)
#	undef  VFS_PLATFORM_WIN
#	define VFS_PLATFORM_WIN	    (1)
#   ifndef WIN32_LEAN_AND_MEAN
#       define WIN32_LEAN_AND_MEAN
#   endif
#   ifndef VC_EXTRALEAN
#       define VC_EXTRALEAN
#   endif
#   ifndef NOMINMAX
#       define NOMINMAX
#   endif
#   include <windows.h>
#elif defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
#	undef  VFS_PLATFORM_POSIX
#	define VFS_PLATFORM_POSIX	(1)
#endif

// Determine if the platform is using Unicode or not
#if defined(UNICODE)
#   define VFS_USE_UNICODE         (1)
#else
#   define VFS_USE_UNICODE         (0)
#endif



#include "vfs/logging.hpp"
