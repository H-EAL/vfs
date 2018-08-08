#pragma once

#include <atomic>
#include <thread>
#include <functional>

#include "vfs/platform.hpp"


namespace vfs {

    //----------------------------------------------------------------------------------------------
    using watcher_impl = class posix_watcher;


    //----------------------------------------------------------------------------------------------
    class posix_watcher
    {
        using callback_t = std::function<void(const path&)>;

    public:
        posix_watcher(const path &dir, const callback_t &callback)
            : running_(false)
            /*, dir_(dir)
            , handle_(INVALID_HANDLE_VALUE)
            , callback_(callback)
            , eventHandle_(nullptr)*/
        {}

        posix_watcher(posix_watcher &&other)
            : running_(other.running_.load())
            /*, dir_(std::move(other.dir_))
            , handle_(other.handle_)
            , callback_(std::move(other.callback_))
            , thread_(std::move(other.thread_))
            , eventHandle_(other.eventHandle_)*/
        {
            //other.handle_       = INVALID_HANDLE_VALUE;
            //other.eventHandle_  = nullptr;
        }

        posix_watcher(const posix_watcher &)                = delete;
        posix_watcher& operator =(const posix_watcher &)    = delete;

        bool startWatching(bool folders, bool files)
        {
            /*
            if (callback_ == nullptr)
            {
                vfs_errorf("NULL callback specified to watcher %ws", dir_.c_str());
                return false;
            }

            const auto bManualReset  = FALSE;
            const auto bInitialState = FALSE;
            eventHandle_ = CreateEvent(nullptr, bManualReset, bInitialState, nullptr);
            if (eventHandle_ == nullptr)
            {
                vfs_errorf("Could not create event for watcher %ws", dir_.c_str());
                return false;
            }

            // Watch the directory for file creation and deletion.
            auto flags = DWORD{ 0 };
            flags |= folders ? FILE_NOTIFY_CHANGE_DIR_NAME : 0;
            flags |= files ? FILE_NOTIFY_CHANGE_FILE_NAME : 0;

            handle_ = FindFirstChangeNotification(dir_.c_str(), FALSE, flags);

            if (handle_ == INVALID_HANDLE_VALUE)
            {
                const auto errorCode = GetLastError();
                vfs_errorf("FindFirstChangeNotification function failed with error %s with specified path %ws", get_last_error_as_string(errorCode).c_str(), dir_.c_str());
                return false;
            }

            running_ = true;
            thread_ = std::thread(
                [this]
            {
                run();
            });
            */
            return true;
        }

        bool stopWatching()
        {
            /*
            running_ = false;
            const auto success = SetEvent(eventHandle_);
            if (success == FALSE)
            {
                vfs_errorf("Could not signal the event to wake up watcher %ws", dir_.c_str());
                return false;
            }
            CloseHandle(eventHandle_);
            eventHandle_ = nullptr;
            */
            return true;
        }

        void wait()
        {
            if (thread_.joinable())
            {
                thread_.join();
            }
        }

    private:
        void run()
        {
            /*
            // Call the callback in case we have some folders in there waiting before we started the process
            callback_(dir_);

            while (running_)
            {
                HANDLE handles[] = { eventHandle_, handle_ };
                const auto dwWaitStatus = WaitForMultipleObjects(2, handles, FALSE, INFINITE);

                if (dwWaitStatus == WAIT_OBJECT_0)
                {
                    vfs_check(running_ == false);
                    return;
                }

                if (dwWaitStatus != (WAIT_OBJECT_0+1) )
                {
                    vfs_check("Unhandled dwWaitStatus {0:x}");
                    return;
                }

                // Call callback
                callback_(dir_);

                if (FindNextChangeNotification(handle_) == FALSE)
                {
                    vfs_errorf("FindNextChangeNotification function failed with error code %d", GetLastError());
                    return;
                }
            }
            */
        }

    private:
        std::atomic<bool>   running_;
        path                dir_;
        //HANDLE              handle_;
        callback_t          callback_;
        std::thread         thread_;
        //HANDLE              eventHandle_;
    };

} /*vfs*/
