#pragma once

#include <sys/mman.h>
#include <stdio.h>

#include "vfs/platform.hpp"
#include "vfs/posix_file_flags.hpp"
#include "vfs/path.hpp"
#include "vfs/posix_move.hpp"


namespace vfs {

    //----------------------------------------------------------------------------------------------
    using file_impl = class posix_file;
    //----------------------------------------------------------------------------------------------


    //----------------------------------------------------------------------------------------------
    class posix_file
        : public posix_move
    {
    protected:
        //------------------------------------------------------------------------------------------
        using native_handle = int32_t;

    protected:
        //------------------------------------------------------------------------------------------
        native_handle nativeHandle() const
        {
            return fileDescriptor_;
        }

        //------------------------------------------------------------------------------------------
        native_handle nativeFileMappingHandle() const
        {
            return -1;
        }

        //------------------------------------------------------------------------------------------
        const path& fileName() const
        {
            return fileName_;
        }

        //------------------------------------------------------------------------------------------
        file_access fileAccess() const
        {
            return fileAccess_;
        }

    protected:
        //------------------------------------------------------------------------------------------
        posix_file
        (
            const path              &name,
            file_access             access,
            file_creation_options   creationOption,
            file_flags              flags,
            file_attributes         attributes
        )
            : fileName_(name)
            , fileDescriptor_(-1)
            , fileAccess_(access)
        {
            auto _creationOption = creationOption;
            
            // There is no equivalent to OPEN_IF_EXISTING in posix. If file doesn't exist we don't open it, if it does we open it with file_creation_options::open_or_create.
            if (creationOption == file_creation_options::open_if_existing)
            {
                if (!exists(name))
                {
                    vfs_errorf("File named %s opened with file_creation_options::open_if_existing doesn't exist.", fileName_.c_str());
                    return;
                }
                else
                {
                    _creationOption = file_creation_options::open_or_create;
                }
            }
           
            const auto _flags =
                posix_file_access(access)                           | 
                posix_file_share_mode(file_share_mode::read)        |
                posix_file_creation_options(_creationOption)        |
                posix_file_flags(flags)                             |
                posix_file_attributes(attributes);
            
            fileDescriptor_ = ::open
            (
                // File name
                fileName_.c_str(),
                // Access
                _flags,
                // Creation mode
                S_IRWXU | S_IRWXG | S_IRWXO
            );

            if (fileDescriptor_  == -1)
            {
                vfs_errorf("open(%s) failed with error: %s", fileName_.c_str(), get_last_error_as_string(errno).c_str());
            }
        }

        //------------------------------------------------------------------------------------------
        ~posix_file()
        {
            close();
        }

    protected:
        //------------------------------------------------------------------------------------------
        static bool exists(const path &filePath)
        {
            struct stat st{};
            stat(filePath.c_str(), &st);
            return (st.st_mode & S_IFMT) == S_IFREG;
        }
        
        //------------------------------------------------------------------------------------------
        static uint64_t get_last_write_time(const path &filePath)
        {
            ///TODO
            return 0ull;
        }

        //------------------------------------------------------------------------------------------
        static void delete_file(const path &filePath)
        {
            if (remove(filePath.c_str()) == -1)
            {
                vfs_errorf("remove(%s) failed with error: %s", filePath.c_str(), get_last_error_as_string(errno).c_str());
            }
        }

    protected:
        //------------------------------------------------------------------------------------------
        bool isValid() const
        {
            return fileDescriptor_ != -1;
        }

        //------------------------------------------------------------------------------------------
        bool isMapped() const
        {
            return false;
        }
        
        //------------------------------------------------------------------------------------------
        void close()
        {
            if (isValid())
            {
                ::close(fileDescriptor_);
                fileDescriptor_ = -1;
            }
        }

        //------------------------------------------------------------------------------------------
        int64_t size() const
        {
            vfs_check(isValid());
            struct stat st;
            stat(fileName_.c_str(), &st);
            return st.st_size;
        }

        //------------------------------------------------------------------------------------------
        bool resize(int64_t newSize)
        {
            vfs_check(isValid());
            
            const auto result = ftruncate64(fileDescriptor_, newSize);

            if (result == -1)
            {
                vfs_errorf("ftruncate(%s) failed with error: %s", fileName_.c_str(), get_last_error_as_string(errno).c_str());
                return false;
            }
            
            return true;
        }

        //------------------------------------------------------------------------------------------
        bool skip(int64_t offset)
        {
            vfs_check(isValid());
            
            const auto resultOffset = lseek64(fileDescriptor_, offset, SEEK_CUR);
            
            if (resultOffset == -1)
            {
                vfs_errorf("lseek64(%s, %ld) failed with error: %s", fileName_.c_str(), offset, get_last_error_as_string(errno).c_str());
                return false;
            }
            
            return true;
        }

        //------------------------------------------------------------------------------------------
        int64_t read(uint8_t *dst, int64_t sizeInBytes)
        {
            vfs_check(isValid());

            const auto numberOfBytesRead = ::read(fileDescriptor_, dst, sizeInBytes);
            
            if (numberOfBytesRead == -1)
            {
                vfs_errorf("::read(%s, %ld) failed with error: %s", fileName_.c_str(), sizeInBytes, get_last_error_as_string(errno).c_str());
                return 0;
            }
            
            return numberOfBytesRead;
        }

        //------------------------------------------------------------------------------------------
        int64_t write(const uint8_t *src, int64_t sizeInBytes)
        {
            vfs_check(isValid());

            const auto numberOfBytesWritten = ::write(fileDescriptor_, src, sizeInBytes);
            
            if (numberOfBytesWritten == -1)
            {
                vfs_errorf("::write(%s, %ld) failed with error: %s", fileName_.c_str(), sizeInBytes, get_last_error_as_string(errno).c_str());
                return 0;
            }
            
            return numberOfBytesWritten;
        }

    private:
       //------------------------------------------------------------------------------------------
        path            fileName_;
        native_handle   fileDescriptor_;
        uint8_t         *pMappedFile_;
        file_access     fileAccess_;
    };
    //----------------------------------------------------------------------------------------------

} /*vfs*/
