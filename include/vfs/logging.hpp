#pragma once

#include <string>
#include <cstring>
#include <cassert>

#define FMT_HEADER_ONLY
#include "fmt/format.h"

#include "vfs/platform.hpp"
#include "vfs/string_utils.hpp"


#ifndef VFS_DISABLE_DEFAULT_ERROR_HANDLING

    #define vfs_infof(MSG, ...)         fmt::print(TEXT("[ info     ] "  MSG  "\n"), ##__VA_ARGS__)
    #define vfs_info(MSG)               vfs_infof(MSG,)

    #define vfs_warningf(MSG, ...)      fmt::print(TEXT("[ warning  ] "  MSG  "\n"), ##__VA_ARGS__)
    #define vfs_warning(MSG)            vfs_warningf(MSG,)

    #define vfs_errorf(MSG, ...)        fmt::print(TEXT("[ error    ] "  MSG  "\n"), ##__VA_ARGS__)
    #define vfs_error(MSG)              vfs_errorf(MSG,)

    #define vfs_criticalf(MSG, ...)     fmt::print(TEXT("[ critical ] "  MSG  "\n"), ##__VA_ARGS__)
    #define vfs_critical(MSG)           vfs_criticalf(MSG,)

    #define vfs_check(EXPR)             assert(EXPR)

#endif // VFS_DISABLE_DEFAULT_ERROR_HANDLING


#if VFS_PLATFORM_WIN

    //----------------------------------------------------------------------------------------------
    // Returns the last Win32 error, in string format. Returns an empty string if there is no error.
    inline vfs::string get_last_error_as_string(DWORD errorCode)
    {
        auto message = vfs::string{};

        // Get the error message, if any.
        if (errorCode != ERROR_SUCCESS)
        {
            using char_ptr_type         = std::conditional_t<VFS_USE_UNICODE, LPWSTR, LPSTR>;
            char_ptr_type messageBuffer = nullptr;
            const auto size             = FormatMessage
            (
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                nullptr,
                errorCode,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (char_ptr_type)&messageBuffer,
                0,
                nullptr
            );

            message = vfs::string(messageBuffer, size);

            LocalFree(messageBuffer);
        }

        return message;
    }
    
#elif VFS_PLATFORM_POSIX

    //----------------------------------------------------------------------------------------------
    inline std::string get_last_error_as_string(int errorCode)
    {
        auto message = std::string{};
        
        if (errorCode != 0)
        {
            constexpr auto MESSAGE_LENGTH = 1024;
            message.resize(MESSAGE_LENGTH);
            strerror_r(errorCode, &message[0], MESSAGE_LENGTH);
        }
        
        return message;
    }

#endif
