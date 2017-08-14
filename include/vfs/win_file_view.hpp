#pragma once

#include "vfs/platform.hpp"
#include "vfs/file.h"


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
            , fileMappingHandle_(nullptr)
            , pData_(nullptr)
            , pCursor_(nullptr)
            , totalSize_(viewSize)
        {
			vfs_check(spFile_->isValid());
            map(viewSize);
        }

		//------------------------------------------------------------------------------------------
        ~win_file_view()
        {
            flush();
            unmap();
        }

		//------------------------------------------------------------------------------------------
        bool map(int64_t viewSize)
        {
			if (!spFile_->createMapping(viewSize))
			{
				return false;
			}

            const auto access = spFile_->fileAccess();
            const auto fileMapAccess = (
                (access == file_access::read_only)
                ? FILE_MAP_READ
                : ((access == file_access::write_only)
                    ? FILE_MAP_WRITE
                    : FILE_MAP_ALL_ACCESS
                )
            );

            pData_ = reinterpret_cast<uint8_t*>(MapViewOfFile(spFile_->nativeFileMappingHandle(), fileMapAccess, 0, 0, 0));
            pCursor_ = pData_;

            if (pData_ == nullptr)
            {
                const auto errorCode = GetLastError();
                vfs_errorf("MapViewOfFile(%ws) failed with error: %s", spFile_->fileName().c_str(), get_last_error_as_string(errorCode).c_str());
                return false;
            }

            MEMORY_BASIC_INFORMATION memInfo;
            const auto dwInfoBytesCount = VirtualQuery(pData_, &memInfo, sizeof(memInfo));
            if (dwInfoBytesCount != 0)
            {
                totalSize_ = memInfo.RegionSize;
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
            return totalSize_;
        }

		//------------------------------------------------------------------------------------------
        bool canMoveCursor(int64_t offsetInBytes) const
        {
            return isValid() && (pCursor_ - pData_ + offsetInBytes) <= totalSize_;
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
        HANDLE      fileMappingHandle_;
        uint8_t     *pData_;
        uint8_t     *pCursor_;
        int64_t     totalSize_;
    };
    //----------------------------------------------------------------------------------------------

} /*vfs*/
