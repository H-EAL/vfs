#pragma once

#include <string>
#include <windows.h>

#include "vfs/stream_interface.hpp"


namespace vizua { namespace filesystem {
    /*
    class file_impl;
    class file_view_impl;

    using file      = stream_interface<file_impl>;
    using file_view = stream_interface<file_view_impl>;
    
    class file_impl
    {
        friend class file_view_impl;
        friend class stream_interface<file_impl>;

    public:
        enum class creation_options : uint32_t
        {
            create_new          = CREATE_NEW,
            create_always       = CREATE_ALWAYS,
            open_existing       = OPEN_EXISTING,
            open_always         = OPEN_ALWAYS,
            truncate_existing   = TRUNCATE_EXISTING
        };

        enum class flags : uint32_t
        {
            none                = 0,
            sequential_scan     = FILE_FLAG_SEQUENTIAL_SCAN,
            delete_on_close     = FILE_FLAG_DELETE_ON_CLOSE
        };

        enum class attributes : uint32_t
        {
            none                = 0,
            normal              = FILE_ATTRIBUTE_NORMAL,
            temporary           = FILE_ATTRIBUTE_TEMPORARY
        };

    private:
        enum class access : uint32_t
        {
            // File creation
            read_only       = GENERIC_READ,
            write_only      = GENERIC_WRITE,
            read_write      = read_only | write_only,
            // File mapping
            map_read_only   = PAGE_READONLY,
            map_read_write  = PAGE_READWRITE
        };

    public:
        static file open_read_only(const std::string &fileName, creation_options creationOptions, flags fileFlags = flags::none, attributes fileAttributes = attributes::normal)
        {
            return create(fileName, access::read_only, creationOptions, fileFlags, fileAttributes);
        }

        static file open_write_only(const std::string &fileName, creation_options creationOptions, flags fileFlags = flags::none, attributes fileAttributes = attributes::normal)
        {
            return create(fileName, access::write_only, creationOptions, fileFlags, fileAttributes);
        }

        static file open_read_write(const std::string &fileName, creation_options creationOptions, flags fileFlags = flags::none, attributes fileAttributes = attributes::normal)
        {
            return create(fileName, access::read_write, creationOptions, fileFlags, fileAttributes);
        }

    public:
        const std::string& getFileName() const
        {
            return fileName_;
        }

        bool isValid() const
        {
            return handle_ != INVALID_HANDLE_VALUE;
        }

        int64_t size() const
        {
            int64_t fileSize = 0;
            GetFileSizeEx(handle_, PLARGE_INTEGER(&fileSize));
            return fileSize;
        }

        int64_t readableSize() const
        {
            return 0;
        }

        int64_t writableSize() const
        {
            return 0;
        }

        int64_t read(uint8_t *dst, int64_t sizeInBytes)
        {
            auto numberOfBytesRead = DWORD{ 0 };
            ReadFile(handle_, (LPVOID)dst, DWORD(sizeInBytes), &numberOfBytesRead, nullptr);
            return numberOfBytesRead;
        }

        int64_t write(const uint8_t *src, int64_t sizeInBytes)
        {
            auto numberOfBytesWritten = DWORD{ 0 };
            WriteFile(handle_, (LPCVOID)src, DWORD(sizeInBytes), &numberOfBytesWritten, nullptr);
            return numberOfBytesWritten;
        }

        void setEndOfFile(int64_t offsetInByteFromBegining)
        {
            auto liDistanceToMove = LARGE_INTEGER{};
            liDistanceToMove.QuadPart = offsetInByteFromBegining;
            SetFilePointerEx(handle_, liDistanceToMove, nullptr, FILE_BEGIN);
            SetEndOfFile(handle_);
        }

        void close()
        {
            if (isMapped())
            {
                unmap();
            }
            if (isValid())
            {
                CloseHandle(handle_);
                handle_ = INVALID_HANDLE_VALUE;
            }
        }

    private:
        file_impl(const std::string &fileName, HANDLE h, access fileAccess)
            : fileName_(fileName)
            , handle_(h)
            , fileMapHandle_(nullptr)
            , fileAccess_(fileAccess)
        {}

        file_impl(file_impl &&other)
            : handle_(other.handle_)
            , fileMapHandle_(other.fileMapHandle_)
            , fileAccess_(other.fileAccess_)
        {
            other.handle_           = INVALID_HANDLE_VALUE;
            other.fileMapHandle_    = nullptr;
        }

    public:
        ~file_impl()
        {
            close();
        }

    private:
        static file create(const std::string &fileName, access desiredAccess, creation_options creationOptions, flags fileFlags, attributes fileAttributes)
        {
            auto handle = CreateFileA(
                fileName.c_str(),
                DWORD(desiredAccess),
                0,
                nullptr,
                DWORD(creationOptions),
                DWORD(fileFlags) | DWORD(fileAttributes),
                nullptr
            );

            if (handle == INVALID_HANDLE_VALUE)
            {
                auto errorCode = GetLastError();
                vizua_errorf("CreateFileA({}) failed with error: {}", fileName, get_last_error_as_string(errorCode));
            }

            return file(fileName, handle, desiredAccess);
        }

    private:
        bool isMapped() const
        {
            return fileMapHandle_ != nullptr;
        }

        bool map(int64_t viewSize)
        {
            if (!isMapped())
            {
                const auto mapAccess = (fileAccess_ == access::read_only) ? access::map_read_only : access::map_read_write;
                fileMapHandle_ = CreateFileMappingA(handle_, nullptr, DWORD(mapAccess), DWORD(viewSize >> 32), DWORD((viewSize << 32) >> 32), nullptr);
                if (fileMapHandle_ == nullptr)
                {
                    auto errorCode = GetLastError();
                    vizua_errorf("CreateFileMappingA({}) failed with error: {}", fileName_, get_last_error_as_string(errorCode));
                }
            }
            return isMapped();
        }

        void unmap()
        {
            if (isMapped())
            {
                CloseHandle(fileMapHandle_);
                fileMapHandle_ = nullptr;
            }
        }

    private:
        const std::string   fileName_;
        HANDLE              handle_;
        HANDLE              fileMapHandle_;
        access              fileAccess_;
    };


    class file_view_impl
    {
    public:
        file_view_impl(file &f, int64_t viewSize)
            : file_(f)
            , pData_(nullptr)
            , pCursor_(nullptr)
            , totalSize_(0)
        {
            file_.map(viewSize);
            if (file_.isMapped())
            {
                const auto fileMapAccess = (
                    (file_.fileAccess_ == file_impl::access::read_only)
                    ? FILE_MAP_READ
                    : ((file_.fileAccess_ == file_impl::access::write_only)
                        ? FILE_MAP_WRITE
                        : FILE_MAP_ALL_ACCESS
                    )
                );
                pData_ = reinterpret_cast<uint8_t*>(MapViewOfFile(file_.fileMapHandle_, fileMapAccess, 0, 0, 0));
                pCursor_ = pData_;

                if (pData_ == nullptr)
                {
                    auto errorCode = GetLastError();
                    vizua_errorf("MapViewOfFile({}) failed with error: {}", file_.getFileName(), get_last_error_as_string(errorCode));
                }

                MEMORY_BASIC_INFORMATION memInfo;
                const auto dwInfoBytesCount = VirtualQuery(pData_, &memInfo, sizeof(memInfo));
                if (dwInfoBytesCount != 0)
                {
                    totalSize_ = memInfo.RegionSize;
                }
            }
        }

        file_view_impl(file &f)
            : file_view_impl(f, 0)
        {}

        const file& getFile() const
        {
            return file_;
        }

        template<typename T = uint8_t>
        T* data()
        {
            return reinterpret_cast<T*>(pData_);
        }

        uint8_t* data()
        {
            return data<>();
        }

        template<typename T = uint8_t>
        T* cursor()
        {
            return reinterpret_cast<T*>(pCursor_);
        }

        uint8_t* cursor()
        {
            return cursor<>();
        }

        bool skip(int64_t offsetInBytes)
        {
            if (canMoveCursor(offsetInBytes))
            {
                pCursor_ += offsetInBytes;
                return true;
            }
            return false;
        }
        
        int64_t read(uint8_t *dst, int64_t sizeInBytes)
        {
            if (canMoveCursor(sizeInBytes))
            {
                memcpy(dst, pCursor_, sizeInBytes);
                pCursor_ += sizeInBytes;
                return sizeInBytes;
            }
            return 0;
        }

        int64_t write(const uint8_t *src, int64_t sizeInBytes)
        {
            if (canMoveCursor(sizeInBytes))
            {
                memcpy(pCursor_, src, sizeInBytes);
                pCursor_ += sizeInBytes;
                return sizeInBytes;
            }
            return 0;
        }

        void flush()
        {
            FlushViewOfFile(pData_, 0);
        }

        void close()
        {
            flush();
            UnmapViewOfFile(pData_);
            file_.unmap();
            pData_ = pCursor_ = nullptr;
        }

        bool isValid() const
        {
            return pData_ != nullptr;
        }

        int64_t totalSize() const
        {
            return totalSize_;
        }

        bool canMoveCursor(int64_t offsetInBytes) const
        {
            return isValid() && (pCursor_ - pData_ + offsetInBytes) <= totalSize_;
        }

    private:
        file    &file_;
        uint8_t *pData_;
        uint8_t *pCursor_;
        int64_t totalSize_;
    };
    */
} /*filesystem*/ } /*vizua*/
