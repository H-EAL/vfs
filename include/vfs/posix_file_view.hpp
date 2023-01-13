#pragma once

#include <cstring>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <limits.h>

#include "vfs/platform.hpp"
#include "vfs/posix_file_flags.hpp"


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
            : sharedMemory_(false)
            , name_(spFile->fileName())
            , fileDescriptor_(spFile->nativeHandle())
            , pData_(nullptr)
            , pCursor_(nullptr)
            , fileTotalSize_(0)
            , mappedTotalSize_(0)
        {
			vfs_check(spFile->isValid());
            map(viewSize, false, spFile->fileAccess());
        }

        //------------------------------------------------------------------------------------------
        posix_file_view(const path &name, int64_t size, bool openExisting)
            : sharedMemory_(true)
            , name_(name)
            , fileDescriptor_(-1)
            , pData_(nullptr)
            , pCursor_(nullptr)
            , fileTotalSize_(0)
            , mappedTotalSize_(0)
        {
            map(size, openExisting, file_access::read_write);
        }

		//------------------------------------------------------------------------------------------
        ~posix_file_view()
        {
            unmap();
            
            if (sharedMemory_ && shm_unlink(name_.c_str()) == -1)
            {
                vfs_errorf("shm_unlink(%s) failed with error: %s", name_.c_str(), get_last_error_as_string(errno).c_str());
            }
        }

		//------------------------------------------------------------------------------------------
        bool map(int64_t viewSize, bool openExisting, const file_access &access)
        {
            const auto protection       = posix_memory_mapping_protection(access);
            auto truncate               = false;

            if (!sharedMemory_)
            {
                // Create a memory mapping of a file (in the file system) in the calling process' virtual address space.
                // Multiple processes can share a view of this same file by calling mmap() with the same file descriptor.
                fileTotalSize_              = calculateCurrentFileSize();
                mappedTotalSize_            = viewSize == 0 ? fileTotalSize_ : viewSize;
                if (fileTotalSize_ < mappedTotalSize_)
                {
                    // Make file bigger.
                    fileTotalSize_  = mappedTotalSize_;
                    truncate        = true;
                }
            }
            else
            {
                // Create or open a shared memory object and map the shared memory object into the virtual address space of the calling process.
                // Shared memory objects are created in a virtual filesystem, normally mounted under /dev/shm. Data will not persist after process ends.
                // Multiple processes can share a view of this same memory by calling shm_open with the same name.

                //https://stackoverflow.com/questions/24875257/why-use-shm-open
                //https://stackoverflow.com/questions/25170795/mmap-file-between-unrelated-processes

                const auto sharedMemoryAccess   = posix_file_access(access);
                // O_CREAT | O_EXCL flags will make shm_open return an error if a shared memory object with the given name already exists. 
                auto flags                      = sharedMemoryAccess | O_CREAT | O_EXCL;
                const auto mode                 = S_IRUSR | S_IWUSR;

                if(!isNameValid(name_))
                {
                    vfs_errorf("The name %s provided is not valid for posix shared memory. Mapping failed.", name_.c_str());
                    return false;
                }

                // name_ will specify the name of the shared-memory object.
                // Processes that wish to access this shared memory must refer to the object by this name.
                fileDescriptor_ = shm_open(name_.c_str(), flags, mode);

                if (openExisting)
                {
                    // Call to shm_open should have failed.
                    const auto shmAlreadyExists = fileDescriptor_ == -1;

                    if (!shmAlreadyExists)
                    {
                        // Then the memory opened didn't exist previously.
                        close(fileDescriptor_);
                        return false;
                    }
                    else if (errno != EEXIST)
                    {
                        vfs_errorf("shm_open() failed with error: %s", get_last_error_as_string(errno).c_str());
                        return false;
                    }
                    else
                    {
                        // Intended behavior.
                        flags           &= ~O_EXCL;
                        fileDescriptor_ = shm_open(name_.c_str(), flags, mode);

                        if (fileDescriptor_ == -1)
                        {
                            vfs_errorf("shm_open() failed with error: %s", get_last_error_as_string(errno).c_str());
                            return false;
                        }

                        fileTotalSize_      = calculateCurrentFileSize();
                        mappedTotalSize_    = viewSize == 0 ? fileTotalSize_ : viewSize;
                    }
                }
                else
                {
                    // We just created a new shared memory object, we need to truncate it to desired view size.
                    fileTotalSize_  = mappedTotalSize_ = viewSize;
                    truncate        = true;
                }
            }

            if (truncate)
            {
                if (ftruncate(fileDescriptor_, fileTotalSize_) == -1)
                {
                    if (sharedMemory_)
                    {
                        close(fileDescriptor_);
                    }
                    vfs_errorf("ftruncate() failed with error %s", get_last_error_as_string(errno).c_str());
                    return false;
                }
            }

            pCursor_ = pData_ = reinterpret_cast<uint8_t *>(mmap(nullptr, mappedTotalSize_, protection, MAP_SHARED, fileDescriptor_, 0));

            if (sharedMemory_)
            {
                // Once mapped into memory, we can close the file descriptor. However we do not want to close a file descriptor possibly still used by spFile_ object.
                close(fileDescriptor_);
            }

            if (pData_ == MAP_FAILED)
            {
                pData_ = pCursor_ = nullptr;
                vfs_errorf("mmap() failed with error: %s", get_last_error_as_string(errno).c_str());
                return false;
            }

            return true;
        }

        //------------------------------------------------------------------------------------------
        int64_t calculateCurrentFileSize() const
        {
            vfs_check(fileDescriptor_ != -1);
            struct stat st;
            const auto result = fstat(fileDescriptor_, &st);
            vfs_check(result != -1);
            return st.st_size;
        }

        //------------------------------------------------------------------------------------------
        bool isNameValid(const std::string &name) const
        {
            // Note that name must be in the form of /somename; that is, a null-terminated string of up to NAME_MAX
            // characters consisting of an initial slash, followed by one or more characters, none of which are slashes.
            return (name.length() < NAME_MAX && name.length() > 1 && name[0] == '/'
                && std::find(name.begin() + 1, name.end(), '/') == name.end());
        }

		//------------------------------------------------------------------------------------------
        bool unmap()
        {
            // munmap will also flush contents back to underlying file if appropriate.
            if (pData_ && munmap(pData_, mappedTotalSize_) == -1)
            {
                vfs_errorf("munmap() failed with error: %s", get_last_error_as_string(errno).c_str());
                return false;
            }
            
            return true;
        }

		//------------------------------------------------------------------------------------------
        bool flush()
        {
            if (!sharedMemory_ && msync(pData_, mappedTotalSize_, MS_ASYNC) == -1)
            {
                vfs_errorf("msync() failed with error: %s", get_last_error_as_string(errno).c_str());
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
            // In windows, mappedTotalSize_ is calculated from VirtualQuery: https://docs.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-virtualquery
            // There is no equivalent for this function in posix.

            // On VirtualQuery:
            // For files that are larger than the address space, you can only map a small portion of the file data at one time. When the first view is complete, you can unmap it and map a new view.
            // To obtain the size of a view, use the VirtualQuery function.

            // For posix this link specifies how memory mapping works for files larger than memory: https://stackoverflow.com/questions/41288602/how-does-a-memory-mapped-file-work-for-files-larger-than-memory
            // If we try to access a file that is so big it cannot fit in virtual memory, we're fucked. (This will probably never happen).
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
        bool        sharedMemory_;
        path        name_;
        uint32_t    fileDescriptor_;
        uint8_t     *pData_;
        uint8_t     *pCursor_;
        int64_t     fileTotalSize_;
        int64_t     mappedTotalSize_;
    };
    //----------------------------------------------------------------------------------------------

} /*vfs*/
