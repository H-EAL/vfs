#pragma once

#include <cstring>
#include <sys/mman.h>

#include "vfs/platform.hpp"


namespace vfs {

    //----------------------------------------------------------------------------------------------
    using file_view_impl = class posix_file_view;
    //----------------------------------------------------------------------------------------------


    //----------------------------------------------------------------------------------------------
    class posix_file_view
    {
	protected:
		//------------------------------------------------------------------------------------------
        posix_file_view(file_sptr spFile, int64_t viewSize)
            : spFile_(std::move(spFile))
            , name_(spFile_->fileName())
            //, fileMappingHandle_(nullptr)
            , pData_(nullptr)
            , pCursor_(nullptr)
            , mappedTotalSize_(viewSize)
        {
			vfs_check(spFile_->isValid());
            map(viewSize, false);
        }

        //------------------------------------------------------------------------------------------
        posix_file_view(const path &name, int64_t size, bool openExisting)
            : name_(name)
            //, fileMappingHandle_(nullptr)
            , pData_(nullptr)
            , pCursor_(nullptr)
            , mappedTotalSize_(size)
        {
            map(size, openExisting);
        }

		//------------------------------------------------------------------------------------------
        ~posix_file_view()
        {
            flush();
            unmap();

            /*
            if (spFile_ == nullptr)
            {
                CloseHandle(fileMappingHandle_);
            }
            */
        }

		//------------------------------------------------------------------------------------------
        bool map(int64_t viewSize, bool openExisting)
        {
            const auto access = spFile_ ? spFile_->fileAccess() : file_access::read_write;

            if (spFile_)
            {
                if (!spFile_->createMapping(viewSize))
                {
                    return false;
                }

                //fileMappingHandle_  = spFile_->nativeFileMappingHandle();
                const auto currentFileSize = spFile_->size();
                fileTotalSize_ = viewSize == 0 ? currentFileSize : viewSize;
                
                if (currentFileSize < viewSize)
                {
                    spFile_->resize(viewSize);
                }
            }
            else
            {
                vfs_check(false);
                /*
                if (openExisting)
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
                    vfs_errorf("%sFileMapping(%ws) failed with error: %s", openExisting ? "Open" : "Create", name_.c_str(), get_last_error_as_string(errorCode).c_str());
                    return false;
                }

                fileTotalSize_ = viewSize;
                */
            }

            const auto fileMapAccess = (
                (access == file_access::read_only)
                ? PROT_READ
                : ((access == file_access::write_only)
                    ? PROT_WRITE
                    : PROT_READ | PROT_WRITE
                )
            );

            pData_ = reinterpret_cast<uint8_t*>(mmap(nullptr, fileTotalSize_, fileMapAccess, MAP_SHARED, spFile_->nativeHandle(), 0));
            pCursor_ = pData_;

            if (pData_ == MAP_FAILED)
            {
                vfs_errorf("mmap(nullptr, %d, %d, MAP_SHARED, %s, 0) failed with error: %s", fileTotalSize_, (int32_t)fileMapAccess, name_.c_str(), get_last_error_as_string(errno).c_str());
                return false;
            }

            return true;
        }

		//------------------------------------------------------------------------------------------
        bool unmap()
        {
            if (pData_ == nullptr || pData_ == MAP_FAILED)
            {
                return true;
            }

            if (munmap(pData_, fileTotalSize_) == -1)
            {
                return false;
            }
            
            return true;
        }

		//------------------------------------------------------------------------------------------
        bool flush()
        {
            /*
            if (!FlushViewOfFile(pData_, 0))
            {
                return false;
            }
            */
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
        //HANDLE      fileMappingHandle_;
        uint8_t     *pData_;
        uint8_t     *pCursor_;
        int64_t     fileTotalSize_;
        int64_t     mappedTotalSize_;
    };
    //----------------------------------------------------------------------------------------------

} /*vfs*/
