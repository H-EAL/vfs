#pragma once

#include <string>
#include <cassert>

#include "vfs/platform.hpp"


#ifndef VFS_DISABLE_DEFAULT_ERROR_HANDLING

    #define vfs_infof(MSG, ...)         fprintf(stderr, "[ info     ] " ## MSG ## "\n", ##__VA_ARGS__)
    #define vfs_info(MSG)               vfs_infof(MSG,)

    #define vfs_warningf(MSG, ...)      fprintf(stderr, "[ warning  ] " ## MSG ## "\n", ##__VA_ARGS__)
    #define vfs_warning(MSG)            vfs_warningf(MSG,)

    #define vfs_errorf(MSG, ...)        fprintf(stderr, "[ error    ] " ## MSG ## "\n", ##__VA_ARGS__)
    #define vfs_error(MSG)              vfs_errorf(MSG,)

    #define vfs_criticalf(MSG, ...)     fprintf(stderr, "[ critical ] " ## MSG ## "\n", ##__VA_ARGS__)
    #define vfs_critical(MSG)           vfs_criticalf(MSG,)

    #define vfs_check(EXPR) assert(EXPR)

#endif // VFS_DISABLE_DEFAULT_ERROR_HANDLING


#if defined(VFS_PLATFORM_WIN)

    //----------------------------------------------------------------------------------------------
    // Returns the last Win32 error, in string format. Returns an empty string if there is no error.
    inline std::string get_last_error_as_string(DWORD errorCode)
    {
        auto message = std::string{};

        // Get the error message, if any.
        if (errorCode != ERROR_SUCCESS)
        {
            LPSTR messageBuffer = nullptr;
            size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

            message = std::string (messageBuffer, size);

            LocalFree(messageBuffer);
        }

        return message;
    }
    //----------------------------------------------------------------------------------------------

#endif // defined(VFS_PLATFORM_WIN) && !defined(VFS_DISABLE_SAFE_WIN_CALLS)
