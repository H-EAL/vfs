#pragma once

#include <fcntl.h>
#include <sys/sendfile.h>
#include <sys/stat.h>


namespace vfs {

    class posix_move
    {
    public:
        //----------------------------------------------------------------------------------------------
        // Moves a file or a directory from src path to dst path.
        // If src path is a directory, src and dst paths must be on the same drive.
        static bool move(const path &src, const path &dst, bool overwrite = false, int32_t maxAttempts = 1)
        {
            // Prefer to try first with rename() since it is atomic.
            // However, rename() only works across the same filesystem so if it fails with EXDEV
            // we know source and target on different filesystems so we can try another method.
            const auto errorCode = rename(src.c_str(), dst.c_str());
            if (errorCode == -1)
            {
                if(errno == EXDEV)
                {
                    return move_across_different_filesystems(src, dst, overwrite);
                }

                vfs_errorf("rename(%s, %s) returned error: %s", src.c_str(), dst.c_str(),
                           get_last_error_as_string(errno).c_str());
                return false;
            }

            return true;
        }

    private:
        //----------------------------------------------------------------------------------------------
        static bool move_across_different_filesystems(const path &src, const path &dst, bool overwrite)
        {
            const auto srcFd = open(src.c_str(), O_RDONLY);
            if(srcFd == -1)
            {
                vfs_errorf("open(%s, O_RDONLY) returned error: %s",
                           src.c_str(), get_last_error_as_string(errno).c_str());
                return false;
            }

            const auto dstFlags = overwrite ? O_WRONLY | O_CREAT | O_TRUNC : O_WRONLY | O_CREAT;
            const auto dstFd    = open(dst.c_str(), dstFlags, S_IRWXU);
            if(dstFd == -1)
            {
                vfs_errorf("open(%s, ...) returned error: %s",
                           dst.c_str(), get_last_error_as_string(errno).c_str());
                return false;
            }

            auto success = true;

            struct stat st;
            stat(src.c_str(), &st);
            off_t offset = 0;

            if(sendfile64(dstFd, srcFd, &offset, st.st_size) == -1)
            {
                vfs_errorf("sendfile64() returned error: %s",
                           get_last_error_as_string(errno).c_str());
                success = false;
            }

            close(srcFd);
            close(dstFd);

            if (remove(src.c_str()) == -1)
            {
                vfs_errorf("remove(%s) returned error code: %s", src.c_str(),
                           get_last_error_as_string(errno).c_str());
                return false;
            }

            return success;
        }
    };

} /*vfs*/
