#pragma once


namespace vfs {

    class posix_move
    {
    public:
        //----------------------------------------------------------------------------------------------
        // Moves a file or a directory from src path to dst path.
        // If src path is a directory, src and dst paths must be on the same drive.
        static bool move(const path &src, const path &dst, bool overwrite = false, int32_t maxAttempts = 1)
        {
            const auto errorCode = rename(src.c_str(), dst.c_str());
            if (errorCode != 0)
            {
                vfs_errorf("rename(%s, %s) returned error: %d", src.c_str(), dst.c_str(), errorCode);
            }
            return true;
        }
    };

} /*vfs*/
