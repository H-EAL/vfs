#pragma once

#include "vfs/platform.hpp"
#include "vfs/file_flags.hpp"
#include "vfs/path.hpp"
#include "vfs/win_move.hpp"
#include "vfs/win_file_flags.hpp"


namespace vfs {

    //----------------------------------------------------------------------------------------------
    using file_impl = class win_file;
    //----------------------------------------------------------------------------------------------


    //----------------------------------------------------------------------------------------------
    class win_file
        : public win_move
    {
    protected:
        using native_handle = HANDLE;

    protected:
        native_handle nativeHandle() const
        {
            return fileHandle_;
        }

        native_handle nativeFileMappingHandle() const
        {
            return fileMappingHandle_;
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
        win_file
        (
            const path              &name,
            file_access             access,
            file_creation_options   creationOption,
            file_flags              flags,
            file_attributes         attributes
        )
            : fileName_(name)
            , fileHandle_(INVALID_HANDLE_VALUE)
            , fileMappingHandle_(nullptr)
            , fileAccess_(access)
        {
            fileHandle_ = CreateFile
            (
                // File name
                fileName_.c_str(),
                // Access
                win_file_access(access),
                // TODO: Add shared access
                win_file_share_mode(file_share_mode::read),
                // Security attributes
                nullptr,
                // Creation options
                win_file_creation_options(creationOption),
                // Flags and attributes
                win_file_flags(flags) | win_file_attributes(attributes),
                // Template file
                nullptr
            );

            if (fileHandle_ == INVALID_HANDLE_VALUE)
            {
                const auto errorCode = GetLastError();
                vfs_errorf("CreateFile(%s) failed with error: %s", fileName_.c_str(), get_last_error_as_string(errorCode).c_str());
            }
        }

        ~win_file()
        {
            close();
        }

    protected:
        static bool exists(const path &filePath)
        {
            const auto dwAttrib = GetFileAttributes(filePath.c_str());
            return (dwAttrib != INVALID_FILE_ATTRIBUTES) && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
        }

        static uint64_t get_last_write_time(const path &filePath)
        {
            auto lastWrite = 0ull;

            auto fileInfo = WIN32_FILE_ATTRIBUTE_DATA{};
            const auto result = GetFileAttributesEx(filePath.c_str(), GetFileExInfoStandard, &fileInfo);

            if (result != 0)
            {
                lastWrite = (uint64_t(fileInfo.ftLastWriteTime.dwHighDateTime) << 32ull) | fileInfo.ftLastWriteTime.dwLowDateTime;
            }

            return lastWrite;
        }

        static void delete_file(const path &filePath)
        {
            if (DeleteFile(filePath.c_str()) == 0)
            {
                const auto errorCode = GetLastError();
                vfs_errorf("DeleteFile(%s) failed with error: %s", filePath.c_str(), get_last_error_as_string(errorCode).c_str());
            }
        }

    protected:
        bool isValid() const
        {
            return fileHandle_ != INVALID_HANDLE_VALUE;
        }

        bool isMapped() const
        {
            return fileMappingHandle_ != nullptr;
        }

        void close()
        {
            closeMapping();
            if (isValid())
            {
                CloseHandle(fileHandle_);
                fileHandle_ = INVALID_HANDLE_VALUE;
            }
        }

        int64_t size() const
        {
            vfs_check(isValid());

            int64_t fileSize = 0;
            if (!GetFileSizeEx(fileHandle_, PLARGE_INTEGER(&fileSize)))
            {
                const auto errorCode = GetLastError();
                vfs_errorf("GetFileSizeEx(%s) failed with error: %s", fileName_.c_str(), get_last_error_as_string(errorCode).c_str());
            }
            return fileSize;
        }

        bool resize()
        {
            vfs_check(isValid());

            if (!SetEndOfFile(fileHandle_))
            {
                const auto errorCode = GetLastError();
                vfs_errorf("SetEndOfFile(%s) failed with error: %s", fileName_.c_str(), get_last_error_as_string(errorCode).c_str());
                return false;
            }
            return true;
        }

        bool skip(int64_t offset)
        {
            vfs_check(isValid());

            auto liDistanceToMove = LARGE_INTEGER{};
            liDistanceToMove.QuadPart = offset;
            if (!SetFilePointerEx(fileHandle_, liDistanceToMove, nullptr, FILE_CURRENT))
            {
                const auto errorCode = GetLastError();
                vfs_errorf("SetFilePointerEx(%s) failed with error: %s", fileName_.c_str(), get_last_error_as_string(errorCode).c_str());
                return false;
            }
            return true;
        }

        int64_t read(uint8_t *dst, int64_t sizeInBytes)
        {
            vfs_check(isValid());

            auto numberOfBytesRead = DWORD{ 0 };
            if (!ReadFile(fileHandle_, (LPVOID)dst, DWORD(sizeInBytes), &numberOfBytesRead, nullptr))
            {
                const auto errorCode = GetLastError();
                vfs_errorf("ReadFile(%s, %d) failed with error: %s", fileName_.c_str(), DWORD(sizeInBytes), get_last_error_as_string(errorCode).c_str());
            }
            return numberOfBytesRead;
        }

        int64_t write(const uint8_t *src, int64_t sizeInBytes)
        {
            vfs_check(isValid());

            auto numberOfBytesWritten = DWORD{ 0 };
            if (!WriteFile(fileHandle_, (LPCVOID)src, DWORD(sizeInBytes), &numberOfBytesWritten, nullptr))
            {
                const auto errorCode = GetLastError();
                vfs_errorf("WriteFile(%s, %d) failed with error: %s", fileName_.c_str(), DWORD(sizeInBytes), get_last_error_as_string(errorCode).c_str());
            }
            return numberOfBytesWritten;
        }

        bool createMapping(int64_t viewSize)
        {
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
                vfs_errorf("CreateFileMapping(%s) failed with error: %s", fileName_.c_str(), get_last_error_as_string(errorCode).c_str());
                return false;
            }

            return true;
        }

        void closeMapping()
        {
            if (isMapped())
            {
                CloseHandle(fileMappingHandle_);
                fileMappingHandle_ = nullptr;
            }
        }

    private:
        path        fileName_;
        HANDLE      fileHandle_;
        HANDLE      fileMappingHandle_;
        file_access fileAccess_;
    };
    //----------------------------------------------------------------------------------------------

} /*vfs*/
