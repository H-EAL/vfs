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
        win_file_view(const path &name, int64_t size, bool openExistent)
            : name_(name)
            , fileMappingHandle_(nullptr)
            , pData_(nullptr)
            , pCursor_(nullptr)
            , mappedTotalSize_(size)
        {
            map(size, openExistent);
        }

		//------------------------------------------------------------------------------------------
        ~win_file_view()
        {
            flush();
            unmap();

            if (spFile_)
            {
                CloseHandle(fileMappingHandle_);
            }
        }

		//------------------------------------------------------------------------------------------
        bool map(int64_t viewSize, bool openExistent)
        {
            const auto access = spFile_ ? spFile_->fileAccess() : file_access::read_write;

            if (spFile_)
            {
                if (!spFile_->createMapping(viewSize))
                {
                    return false;
                }

                fileMappingHandle_  = spFile_->nativeFileMappingHandle();
                fileTotalSize_      = spFile_->size();
            }
            else
            {
                if (openExistent)
                {
                    fileMappingHandle_ = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, name_.c_str());
                }
                else
                {
                    fileMappingHandle_ = CreateFileMapping
                    (
                        INVALID_HANDLE_VALUE,
                        nullptr,
                        PAGE_READWRITE,
                        DWORD(viewSize >> 32), DWORD((viewSize << 32) >> 32),
                        name_.c_str()
                    );
                }

                if (fileMappingHandle_ == nullptr)
                {
                    const auto errorCode = GetLastError();
                    vfs_errorf("%sFileMapping(%ws) failed with error: %s", openExistent ? "Open" : "Create", name_.c_str(), get_last_error_as_string(errorCode).c_str());
                    return false;
                }

                fileTotalSize_ = viewSize;
            }

            const auto fileMapAccess = (
                (access == file_access::read_only)
                ? FILE_MAP_READ
                : ((access == file_access::write_only)
                    ? FILE_MAP_WRITE
                    : FILE_MAP_ALL_ACCESS
                )
            );

            pData_ = reinterpret_cast<uint8_t*>(MapViewOfFile(fileMappingHandle_, fileMapAccess, 0, 0, 0));
            pCursor_ = pData_;

            if (pData_ == nullptr)
            {
                const auto errorCode = GetLastError();
                vfs_errorf("MapViewOfFile(%ws) failed with error: %s", name_.c_str(), get_last_error_as_string(errorCode).c_str());
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
