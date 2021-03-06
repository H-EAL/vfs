#pragma once

#include "vfs/platform.hpp"


namespace vfs {

    //----------------------------------------------------------------------------------------------
    using file_view_impl = class win_file_view;
    //----------------------------------------------------------------------------------------------


    //----------------------------------------------------------------------------------------------
    class win_file_view
    {
	protected:
		//------------------------------------------------------------------------------------------
        win_file_view(file_sptr spFile, int64_t viewSize)
            : spFile_(std::move(spFile))
            , name_(spFile_->fileName())
            , fileMappingHandle_(nullptr)
            , pData_(nullptr)
            , pCursor_(nullptr)
            , mappedTotalSize_(viewSize)
        {
			vfs_check(spFile_->isValid());
            map(viewSize, false);
        }

        //------------------------------------------------------------------------------------------
        win_file_view(const path &name, int64_t size, bool openExisting)
            : name_(name)
            , fileMappingHandle_(nullptr)
            , pData_(nullptr)
            , pCursor_(nullptr)
            , mappedTotalSize_(size)
        {
            map(size, openExisting);
        }

		//------------------------------------------------------------------------------------------
        ~win_file_view()
        {
            flush();
            unmap();
            CloseHandle(fileMappingHandle_);
        }

		//------------------------------------------------------------------------------------------
        bool map(int64_t viewSize, bool openExisting)
        {
            const auto access = spFile_ ? spFile_->fileAccess() : file_access::read_write;

            if (openExisting)
            {
                fileMappingHandle_ = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, name_.c_str());
            }
            else
            {
                const auto fileHandle   = spFile_ ? spFile_->nativeHandle() : INVALID_HANDLE_VALUE;
                const auto protect      = DWORD((access == file_access::read_only) ? PAGE_READONLY : PAGE_READWRITE);

                fileMappingHandle_ = CreateFileMapping
                (
                    // Handle to the file to map
                    fileHandle,
                    // Security attributes
                    nullptr,
                    // Page protection level
                    protect /*| SEC_RESERVE*/,                           //SEC_RESERVE allows extensions on shared memory cases
                    // Mapping size, whole file if 0
                    DWORD(viewSize >> 32), DWORD((viewSize << 32) >> 32),
                    // File mapping object name for system wide objects
                    spFile_ ? nullptr : name_.c_str()
                );
            }

            fileTotalSize_ = spFile_ ? spFile_->size() : viewSize;

            if (fileMappingHandle_ == nullptr)
            {
                const auto errorCode = GetLastError();
                vfs_errorf("%sFileMapping(%s) failed with error: %s", openExisting ? "Open" : "Create", name_.c_str(), get_last_error_as_string(errorCode).c_str());
                return false;
            }


            const auto fileMapAccess = (
                (access == file_access::read_only)
                ? FILE_MAP_READ
                : ((access == file_access::write_only)
                    ? FILE_MAP_WRITE
                    : FILE_MAP_ALL_ACCESS
                )
            );

            // map only allocated size, and allow extension on shared memory
            pData_ = reinterpret_cast<uint8_t*>(MapViewOfFile(fileMappingHandle_, fileMapAccess, 0, 0, viewSize));
            pCursor_ = pData_;

            if (pData_ == nullptr)
            {
                const auto errorCode = GetLastError();
                vfs_errorf("MapViewOfFile(%s) failed with error: %s", name_.c_str(), get_last_error_as_string(errorCode).c_str());
                return false;
            }

            MEMORY_BASIC_INFORMATION memInfo;
            const auto dwInfoBytesCount = VirtualQuery(pData_, &memInfo, sizeof(memInfo));
            if (dwInfoBytesCount != 0)
            {
                mappedTotalSize_ = memInfo.RegionSize;
            }

            return true;
        }

		//------------------------------------------------------------------------------------------
        bool unmap()
        {
            if (pData_ && !UnmapViewOfFile(pData_))
            {
                return false;
            }
            return true;
        }

		//------------------------------------------------------------------------------------------
        bool flush()
        {
            if (!FlushViewOfFile(pData_, 0))
            {
                return false;
            }
            return true;
        }

		//------------------------------------------------------------------------------------------
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

		//------------------------------------------------------------------------------------------
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

		//------------------------------------------------------------------------------------------
        bool isValid() const
        {
            return pData_ != nullptr;
        }

		//------------------------------------------------------------------------------------------
        int64_t totalSize() const
        {
            return fileTotalSize_;
        }

		//------------------------------------------------------------------------------------------
        bool canMoveCursor(int64_t offsetInBytes) const
        {
            return isValid() && (pCursor_ - pData_ + offsetInBytes) <= mappedTotalSize_;
        }

		//------------------------------------------------------------------------------------------
        template<typename T = uint8_t>
        T* data()
        {
            return reinterpret_cast<T*>(pData_);
        }

		//------------------------------------------------------------------------------------------
        uint8_t* data()
        {
            return data<>();
        }

		//------------------------------------------------------------------------------------------
        template<typename T = uint8_t>
        T* cursor()
        {
            return reinterpret_cast<T*>(pCursor_);
        }

		//------------------------------------------------------------------------------------------
        bool skip(int64_t offsetInBytes)
        {
            if (canMoveCursor(offsetInBytes))
            {
                pCursor_ += offsetInBytes;
                return true;
            }
            return false;
        }

		//------------------------------------------------------------------------------------------
        auto getFile() const
        {
            return spFile_;
        }

	private:
		//------------------------------------------------------------------------------------------
        file_sptr   spFile_;
        path        name_;
        HANDLE      fileMappingHandle_;
        uint8_t     *pData_;
        uint8_t     *pCursor_;
        int64_t     fileTotalSize_;
        int64_t     mappedTotalSize_;
    };
    //----------------------------------------------------------------------------------------------

} /*vfs*/
