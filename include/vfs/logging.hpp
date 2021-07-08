#pragma once

#include <string>
#include <cstring>
#include <cassert>

#include "vfs/platform.hpp"


#ifndef VFS_DISABLE_DEFAULT_ERROR_HANDLING

    #define vfs_infof(MSG, ...)         fprintf(stderr, "[ info     ] "  MSG  "\n", ##__VA_ARGS__)
    #define vfs_info(MSG)               vfs_infof(MSG,)

    #define vfs_warningf(MSG, ...)      fprintf(stderr, "[ warning  ] "  MSG  "\n", ##__VA_ARGS__)
    #define vfs_warning(MSG)            vfs_warningf(MSG,)

    #define vfs_errorf(MSG, ...)        fprintf(stderr, "[ error    ] "  MSG  "\n", ##__VA_ARGS__)
    #define vfs_error(MSG)              vfs_errorf(MSG,)

    #define vfs_criticalf(MSG, ...)     fprintf(stderr, "[ critical ] "  MSG  "\n", ##__VA_ARGS__)
    #define vfs_critical(MSG)           vfs_criticalf(MSG,)

    #define vfs_check(EXPR)             assert(EXPR)

#endif // VFS_DISABLE_DEFAULT_ERROR_HANDLING


#if VFS_PLATFORM_WIN

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
    
#elif VFS_PLATFORM_POSIX

    //----------------------------------------------------------------------------------------------
    inline std::string get_last_error_as_string(int errorCode)
    {
        auto message = std::string{};
        
        if (errorCode != 0)
        {
            constexpr auto MESSAGE_LENGTH = 1024;
            message.resize(MESSAGE_LENGTH);

            // XSI-compliant strerror_r() does not work with clang and g++/
            // We must use the GNU-specific strerror_r() which returns a char* rather than int.
            // GNU-specific strerror_r() will either fill the buffer we provide and return a pointer to a string that the function stores in the buffer, or return a pointer to some immutable static string.
            char *staticErrorMessage = strerror_r(errorCode, &message[0], MESSAGE_LENGTH);
            strncpy(&message[0], staticErrorMessage, MESSAGE_LENGTH);
        }
        
        return message;
    }

#endif
