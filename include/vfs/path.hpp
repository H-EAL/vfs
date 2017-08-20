#pragma once

#include <string>


namespace vfs {

    //----------------------------------------------------------------------------------------------
    using path = std::wstring;

    //----------------------------------------------------------------------------------------------
    inline path combine(const path &lhs, const path &rhs)
    {
        if (!lhs.empty() && (lhs[lhs.length()] == L'\\' || lhs[lhs.length()] == L'/'))
        {
            return lhs + rhs;
        }
        return lhs + L'\\' + rhs;
    }

    //----------------------------------------------------------------------------------------------
    inline bool move(const path &src, const path &dst, bool overwrite = false, int32_t maxAttempts = 1)
    {
        // When moving a file, the destination can be on a different file system or volume.
        // If the destination is on another drive, you must set the MOVEFILE_COPY_ALLOWED flag in dwFlags.
        auto flags = MOVEFILE_COPY_ALLOWED | MOVEFILE_WRITE_THROUGH;
        if (overwrite)
        {
            flags |= MOVEFILE_REPLACE_EXISTING;
        }

        auto attempts = 0;
        while (MoveFileEx(src.c_str(), dst.c_str(), flags) == FALSE)
        {
            if (++attempts == maxAttempts)
            {
                const auto errorCode = GetLastError();
                vfs_errorf("MoveFileEx(%ws, %ws, MOVEFILE_COPY_ALLOWED | MOVEFILE_WRITE_THROUGH) failed after %d attempts, returned error: %s", src.c_str(), dst.c_str(), attempts, get_last_error_as_string(errorCode).c_str());
                return false;
            }
        }

        return true;
    }

} /*vfs*/
