#pragma once

#include "vfs/platform.hpp"
#include "vfs/file_flags.hpp"
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
        using native_handle = int32_t;

    protected:
        native_handle nativeHandle() const
        {
            return fileHandle_;
        }

        native_handle nativeFileMappingHandle() const
        {
            return -1;// fileMappingHandle_;
        }

        const path& fileName() const
        {
            return fileName_;
        }

        file_access fileAccess() const
        {
            return fileAccess_;
        }

    protected:
        posix_file
        (
            const path              &name,
            file_access             access,
            file_creation_options   creationOption,
            file_flags              flags,
            file_attributes         attributes
        )
            : fileName_(name)
            , fileHandle_(-1)
            //, fileMappingHandle_(nullptr)
            , fileAccess_(access)
        {
            const auto _flags =
                posix_file_access(access)                     | 
                posix_file_share_mode(file_share_mode::read)  |
                posix_file_creation_options(creationOption)   |
                posix_file_flags(flags)                       |
                posix_file_attributes(attributes);
            
            fileHandle_ = ::open
            (
                // File name
                fileName_.c_str(),
                // Access
                _flags,
                // Creation mode
                S_IRWXU | S_IRWXG | S_IRWXO
            );

            if (fileHandle_  == -1)
            {
                const auto errorCode = errno;
                vfs_errorf("open(%s) failed with error: %s", fileName_.c_str(), get_last_error_as_string(errorCode).c_str());
            }
        }

        ~posix_file()
        {
            close();
        }

    protected:
        static bool exists(const path &filePath)
        {
            struct stat st{};
            stat(filePath.c_str(), &st);
            return (st.st_mode & S_IFMT) == S_IFREG;
        }
        
        static uint64_t get_last_write_time(const path &filePath)
        {
            ///TODO
            return 0ull;
        }

    protected:
        bool isValid() const
        {
            return fileHandle_ >= 0;
        }

        bool isMapped() const
        {
            //return fileMappingHandle_ != nullptr;
            return false;
        }

        void close()
        {
            closeMapping();
            if (isValid())
            {
                ::close(fileHandle_);
                fileHandle_ = -1;
            }
        }

        int64_t size() const
        {
            vfs_check(isValid());
            struct stat st;
            stat(fileName_.c_str(), &st);
            return st.st_size;
        }

        bool resize(int64_t newSize)
        {
            vfs_check(isValid());
            
            const auto result = ftruncate64(fileHandle_, newSize);

            if (result == -1)
            {
                vfs_errorf("ftruncate(%s) failed with error: %s", fileName_.c_str(), get_last_error_as_string(errno).c_str());
                return false;
            }
            
            return true;
        }

        bool skip(int64_t offset)
        {
            vfs_check(isValid());
            
            const auto resultOffset = lseek64(fileHandle_, offset, SEEK_CUR);
            
            if (resultOffset == -1)
            {
                vfs_errorf("lseek64(%s, %ld) failed with error: %s", fileName_.c_str(), offset, get_last_error_as_string(errno).c_str());
                return false;
            }
            
            return true;
        }

        int64_t read(uint8_t *dst, int64_t sizeInBytes)
        {
            vfs_check(isValid());

            const auto numberOfBytesRead = ::read(fileHandle_, dst, sizeInBytes);
            
            if (numberOfBytesRead == -1)
            {
                vfs_errorf("::read(%s, %ld) failed with error: %s", fileName_.c_str(), sizeInBytes, get_last_error_as_string(errno).c_str());
                return 0;
            }
            
            return numberOfBytesRead;
        }

        int64_t write(const uint8_t *src, int64_t sizeInBytes)
        {
            vfs_check(isValid());

            const auto numberOfBytesWritten = ::write(fileHandle_, src, sizeInBytes);
            
            if (numberOfBytesWritten == -1)
            {
                vfs_errorf("::write(%s, %ld) failed with error: %s", fileName_.c_str(), sizeInBytes, get_last_error_as_string(errno).c_str());
                return 0;
            }
            
            return numberOfBytesWritten;
        }

        bool createMapping(int64_t viewSize)
        {
            /*/
            const auto protect = DWORD((fileAccess_ == file_access::read_only) ? PAGE_READONLY : PAGE_READWRITE);
            fileMappingHandle_ = CreateFileMapping
            (
                // Handle to the file to map
                fileHandle_,
                // Security attributes
                nullptr,
                // Page protection level
                protect,
                // Mapping size, whole file if 0
                DWORD(viewSize >> 32), DWORD((viewSize << 32) >> 32),
                // File mapping object name for system wide objects, unsupported for now
                nullptr
            );

            if (fileMappingHandle_ == nullptr)
            {
                const auto errorCode = GetLastError();
                vfs_errorf("CreateFileMapping(%ws) failed with error: %s", fileName_.c_str(), get_last_error_as_string(errorCode).c_str());
                return false;
            }
            */
            return true;
        }

        void closeMapping()
        {
            /*
            if (isMapped())
            {
                CloseHandle(fileMappingHandle_);
                fileMappingHandle_ = nullptr;
            }
            */
        }

    private:
        path            fileName_;
        native_handle   fileHandle_;
        //HANDLE          fileMappingHandle_;
        file_access     fileAccess_;

    private:
        static int32_t posix_file_access(file_access access)
        {
            switch (access)
            {
            case file_access::read_only:    return O_RDONLY;
            case file_access::write_only:   return O_WRONLY;
            case file_access::read_write:   return O_RDWR;
            }

            vfs_check(false);
            return 0;
		}

		static int32_t posix_file_share_mode(file_share_mode shareMode)
		{
            /*
			switch (shareMode)
			{
			case file_share_mode::exclusive:	return 0;
			case file_share_mode::can_delete:	return FILE_SHARE_DELETE;
			case file_share_mode::read:			return FILE_SHARE_READ;
			case file_share_mode::write:		return FILE_SHARE_WRITE;
			}

			vfs_check(false);
            */
			return 0;
		}

        static int32_t posix_file_creation_options(file_creation_options creationOption)
        {
            switch (creationOption)
            {
            case file_creation_options::create_if_nonexisting:  return O_CREAT | O_EXCL;
            case file_creation_options::create_or_overwrite:    return O_CREAT | O_TRUNC;
            case file_creation_options::open_if_existing:       return O_APPEND;
            case file_creation_options::open_or_create:         return O_CREAT | O_APPEND;
            case file_creation_options::truncate_existing:      return O_TRUNC;
            }

            vfs_check(false);
            return 0;
        }

        static int32_t posix_file_flags(file_flags flags)
        {
            auto f = int32_t{ 0 };
            /*
            if (uint32_t(flags) & uint32_t(file_flags::delete_on_close))
                f |= FILE_FLAG_DELETE_ON_CLOSE;

            if (uint32_t(flags) & uint32_t(file_flags::sequential_scan))
                f |= FILE_FLAG_SEQUENTIAL_SCAN;
            */
            return f;
        }

        static int32_t posix_file_attributes(file_attributes attributes)
        {
            auto attr = int32_t{ 0 };
            /*
            if (uint32_t(attributes) & uint32_t(file_attributes::normal))
                attr |= FILE_ATTRIBUTE_NORMAL;
            
            if (uint32_t(attributes) & uint32_t(file_attributes::temporary))
                attr |= O_TMPFILE;
            */
            return attr;
        }
    };
    //----------------------------------------------------------------------------------------------

} /*vfs*/
