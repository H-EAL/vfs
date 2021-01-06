#pragma once


namespace vfs {

    class win_move
    {
    public:
        //----------------------------------------------------------------------------------------------
        // Moves a file or a directory from src path to dst path.
        // If src path is a directory, src and dst paths must be on the same drive.
        static bool move(const path &src, const path &dst, bool overwrite = false, int32_t maxAttempts = 1)
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
                    vfs_errorf("MoveFileEx({}, {}, MOVEFILE_COPY_ALLOWED | MOVEFILE_WRITE_THROUGH) failed after {} attempts, returned error: {}",
                        src.str(), dst.str(), attempts, get_last_error_as_string(errorCode));
                    return false;
                }
            }
            return true;
        }

        //----------------------------------------------------------------------------------------------
        // Moves a file or a directory from src path to dst path.
        // If src path is a directory, src and dst paths must be on the same drive.
        static bool copy(const path &src, const path &dst, bool overwrite = false, int32_t maxAttempts = 1)
        {
            // When moving a file, the destination can be on a different file system or volume.
            // If the destination is on another drive, you must set the MOVEFILE_COPY_ALLOWED flag in dwFlags.
            auto flags = MOVEFILE_COPY_ALLOWED | MOVEFILE_WRITE_THROUGH;
            if (overwrite)
            {
                flags |= MOVEFILE_REPLACE_EXISTING;
            }

            auto attempts = 0;
            while (CopyFile(src.c_str(), dst.c_str(), overwrite ? FALSE : TRUE) == FALSE)
            {
                if (++attempts == maxAttempts)
                {
                    const auto errorCode = GetLastError();
                    vfs_errorf("CopyFile({}, {}, {}) failed after {} attempts, returned error: {}",
                        src.str(), dst.str(), overwrite, attempts, get_last_error_as_string(errorCode));
                    return false;
                }
            }
            return true;
        }
    };

} /*vfs*/
