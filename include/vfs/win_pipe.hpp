#pragma once

#include <chrono>

#include "vfs/platform.hpp"
#include "vfs/win_file_flags.hpp"
#include "vfs/path.hpp"


namespace vfs {

    //----------------------------------------------------------------------------------------------
    using pipe_impl = class win_pipe;


    //----------------------------------------------------------------------------------------------
    class win_pipe
    {
    protected:
        //------------------------------------------------------------------------------------------
        using native_handle = HANDLE;

    protected:
        //------------------------------------------------------------------------------------------
        native_handle nativeHandle() const
        {
            return pipeHandle_;
        }

        //------------------------------------------------------------------------------------------
        const path& fileName() const
        {
            return pipeName_;
        }

        //------------------------------------------------------------------------------------------
        file_access fileAccess() const
        {
            return fileAccess_;
        }

    protected:
        //------------------------------------------------------------------------------------------
        win_pipe
        (
            const path              &name,
            file_access             access,
            file_flags              flags,
            file_attributes         attributes
        )
            : pipeName_(name)
            , pipeHandle_(INVALID_HANDLE_VALUE)
            , fileAccess_(access)
        {
            if (!waitForPipe(NMPWAIT_WAIT_FOREVER))
            {
                return;
            }

            pipeHandle_ = CreateFile
            (
                // File name
                pipeName_.c_str(),
                // Access
                win_file_access(access),
                // TODO: Add shared access
                win_file_share_mode(file_share_mode::read),
                // Security attributes
                nullptr,
                // Creation options
                win_file_creation_options(file_creation_options::open_if_existing),
                // Flags and attributes
                win_file_flags(flags) | win_file_attributes(attributes),
                // Template file
                nullptr
            );

            if (pipeHandle_ == INVALID_HANDLE_VALUE)
            {
                const auto errorCode = GetLastError();
                vfs_errorf("CreateFile(%s) failed with error: %s", pipeName_.c_str(), get_last_error_as_string(errorCode).c_str());
            }
        }

        //------------------------------------------------------------------------------------------
        win_pipe
        (
            const path &name
        )
            : pipeName_(name)
        {
            pipeHandle_ = CreateNamedPipe
            (
                pipeName_.c_str(),
                PIPE_ACCESS_INBOUND,
                PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
                PIPE_UNLIMITED_INSTANCES,
                0, 4096,
                NMPWAIT_USE_DEFAULT_WAIT,
                nullptr
            );

            if (pipeHandle_ == INVALID_HANDLE_VALUE)
            {
                const auto errorCode = GetLastError();
                vfs_errorf("CreateNamedPipe failed with error: %s", get_last_error_as_string(errorCode).c_str());
            }
        }

        //------------------------------------------------------------------------------------------
        ~win_pipe()
        {
            close();
        }

    protected:
        //------------------------------------------------------------------------------------------
        bool isValid() const
        {
            return pipeHandle_ != INVALID_HANDLE_VALUE;
        }

        //------------------------------------------------------------------------------------------
        void close()
        {
            if (isValid())
            {
                CloseHandle(pipeHandle_);
                pipeHandle_ = INVALID_HANDLE_VALUE;
            }
        }

        //------------------------------------------------------------------------------------------
        bool waitForPipe(DWORD timeoutInMs)
        {
            if (WaitNamedPipe(pipeName_.c_str(), timeoutInMs) == FALSE)
            {
                const auto errorCode = GetLastError();
                vfs_errorf("WaitNamedPipe failed with error: %s", get_last_error_as_string(errorCode).c_str());
                return false;
            }

            return true;
        }

        //------------------------------------------------------------------------------------------
        template<typename _Rep, typename _Period>
        bool waitForPipe(std::chrono::duration<_Rep, _Period> duration)
        {
            const auto timeoutInMs = DWORD(std::chrono::duration_cast<std::chrono::milliseconds>(duration));
            waitForPipe(timeoutInMs);
        }

        //------------------------------------------------------------------------------------------
        bool waitForConnection()
        {
            if (ConnectNamedPipe(pipeHandle_, nullptr) == FALSE)
            {
                const auto errorCode = GetLastError();
                vfs_errorf("ConnectNamedPipe failed with error: %s", get_last_error_as_string(errorCode).c_str());
                return false;
            }

            return true;
        }

        //------------------------------------------------------------------------------------------
        int64_t availableBytesToRead() const
        {
            auto availableBytes = DWORD(0);
            if (!PeekNamedPipe(pipeHandle_, nullptr, 0, nullptr, nullptr, &availableBytes))
            {
                const auto errorCode = GetLastError();
                vfs_errorf("PeekNamedPipe failed with error: %s", get_last_error_as_string(errorCode).c_str());
            }
            return int64_t(availableBytes);
        }

        //------------------------------------------------------------------------------------------
        int64_t read(uint8_t *dst, int64_t sizeInBytes)
        {
            vfs_check(isValid());

            auto numberOfBytesRead = DWORD{ 0 };
            if (!ReadFile(pipeHandle_, (LPVOID)dst, DWORD(sizeInBytes), &numberOfBytesRead, nullptr))
            {
                const auto errorCode = GetLastError();
                if (errorCode != ERROR_MORE_DATA)
                {
                    vfs_errorf("ReadFile(%s, %d) failed with error: %s", pipeName_.c_str(), DWORD(sizeInBytes), get_last_error_as_string(errorCode).c_str());
                }
            }
            return numberOfBytesRead;
        }

        //------------------------------------------------------------------------------------------
        int64_t write(const uint8_t *src, int64_t sizeInBytes)
        {
            vfs_check(isValid());

            auto numberOfBytesWritten = DWORD{ 0 };
            if (!WriteFile(pipeHandle_, (LPCVOID)src, DWORD(sizeInBytes), &numberOfBytesWritten, nullptr))
            {
                const auto errorCode = GetLastError();
                vfs_errorf("WriteFile(%s, %d) failed with error: %s", pipeName_.c_str(), DWORD(sizeInBytes), get_last_error_as_string(errorCode).c_str());
            }
            return numberOfBytesWritten;
        }

    private:
        //------------------------------------------------------------------------------------------
        path        pipeName_;
        HANDLE      pipeHandle_;
        file_access fileAccess_;
    };

} /*vfs*/
