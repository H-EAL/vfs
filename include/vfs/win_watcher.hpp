#pragma once

#include <atomic>
#include <thread>
#include <chrono>
#include <functional>

#include "vfs/platform.hpp"


namespace vfs {

    //----------------------------------------------------------------------------------------------
    using watcher_impl = class win_watcher;


    //----------------------------------------------------------------------------------------------
    class win_watcher
    {
        //------------------------------------------------------------------------------------------
        using callback_t = std::function<void(const path&)>;

    public:
        //------------------------------------------------------------------------------------------
        template<typename R, typename P>
        win_watcher(const path &dir, std::chrono::duration<R, P> waitTimeout, const callback_t &callback)
            : running_(false)
            , dir_(dir)
            , changeHandle_(INVALID_HANDLE_VALUE)
            , callback_(callback)
            , eventHandle_(nullptr)
            , waitTimeoutInMs_(DWORD(std::chrono::duration_cast<std::chrono::milliseconds>(waitTimeout).count()))
            , filter_(0)
        {
            waitTimeoutInMs_ = (waitTimeoutInMs_ == 0) ? INFINITE : waitTimeoutInMs_;
        }

        //------------------------------------------------------------------------------------------
        win_watcher(const path &dir, const callback_t &callback)
            : win_watcher(dir, std::chrono::seconds(0), callback)
        {}

        //------------------------------------------------------------------------------------------
        win_watcher(win_watcher &&other)
            : running_(other.running_.load())
            , dir_(std::move(other.dir_))
            , changeHandle_(other.changeHandle_)
            , callback_(std::move(other.callback_))
            , thread_(std::move(other.thread_))
            , eventHandle_(other.eventHandle_)
            , waitTimeoutInMs_(other.waitTimeoutInMs_)
            , filter_(other.filter_)
        {
            other.changeHandle_ = INVALID_HANDLE_VALUE;
            other.eventHandle_  = nullptr;
        }

        //------------------------------------------------------------------------------------------
        win_watcher(const win_watcher &)                = delete;
        win_watcher& operator =(const win_watcher &)    = delete;

        //------------------------------------------------------------------------------------------
        bool startWatching(bool folders, bool files)
        {
            if (callback_ == nullptr)
            {
                vfs_errorf("NULL callback specified to watcher %s", dir_.c_str());
                return false;
            }

            const auto bManualReset  = FALSE;
            const auto bInitialState = FALSE;
            eventHandle_ = CreateEvent(nullptr, bManualReset, bInitialState, nullptr);
            if (eventHandle_ == nullptr)
            {
                vfs_errorf("Could not create event for watcher %s", dir_.c_str());
                return false;
            }

            // Watch the directory for file creation and deletion.
            filter_ |= folders ? FILE_NOTIFY_CHANGE_DIR_NAME : 0;
            filter_ |= files ? FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE : 0;

            if (!setupHandle())
            {
                return false;
            }

            running_ = true;
            thread_ = std::thread(
                [this]
            {
                run();
            });

            return true;
        }

        //------------------------------------------------------------------------------------------
        bool setupHandle()
        {
            changeHandle_ = FindFirstChangeNotification(dir_.c_str(), FALSE, filter_);

            if (changeHandle_ == INVALID_HANDLE_VALUE)
            {
                const auto errorCode = GetLastError();
                vfs_errorf("FindFirstChangeNotification function failed with error %s with specified path %s", get_last_error_as_string(errorCode).c_str(), dir_.c_str());
                return false;
            }

            return true;
        }

        //------------------------------------------------------------------------------------------
        bool stopWatching()
        {
            running_ = false;
            wakeUp();
            CloseHandle(eventHandle_);
            eventHandle_ = nullptr;
            return true;
        }

        //------------------------------------------------------------------------------------------
        void wakeUp()
        {
            const auto success = SetEvent(eventHandle_);
            if (success == FALSE)
            {
                vfs_errorf("Could not signal the event to wake up watcher %s", dir_.c_str());
            }
        }

        //------------------------------------------------------------------------------------------
        void wait()
        {
            if (thread_.joinable())
            {
                thread_.join();
            }
        }

    private:
        //------------------------------------------------------------------------------------------
        void run()
        {
            // Call the callback in case we have some folders in there waiting before we started the process
            callback_(dir_);

            while (running_)
            {
                HANDLE handles[] = { eventHandle_, changeHandle_ };
                const auto dwWaitStatus = WaitForMultipleObjects(2, handles, FALSE, waitTimeoutInMs_);

                switch (dwWaitStatus)
                {
                case WAIT_OBJECT_0:
                    if (running_ == false)
                    {
                        return;
                    }
                case WAIT_OBJECT_0 + 1:
                case WAIT_TIMEOUT:
                    break;

                default:
                    vfs_errorf("Unhandled dwWaitStatus %x.", dwWaitStatus);
                    return;
                }

                // Call callback
                callback_(dir_);

                static constexpr auto max_attempts = 5;
                auto attempts = max_attempts;
                while (FindNextChangeNotification(changeHandle_) == FALSE)
                {
                    const auto errorCode = GetLastError();
                    vfs_warningf("FindNextChangeNotification function failed with error code %s.", get_last_error_as_string(errorCode).c_str());
                    std::this_thread::sleep_for(std::chrono::seconds(1));

                    FindCloseChangeNotification(changeHandle_);

                    if (!setupHandle())
                    {
                        return;
                    }

                    if (--attempts == 0)
                    {
                        vfs_errorf("FindNextChangeNotification kept failing afer %d attempts.", max_attempts);
                        return;
                    }
                }
            }

            FindCloseChangeNotification(changeHandle_);
        }

    private:
        //------------------------------------------------------------------------------------------
        std::atomic<bool>   running_;
        path                dir_;
        HANDLE              changeHandle_;
        callback_t          callback_;
        std::thread         thread_;
        HANDLE              eventHandle_;
        DWORD               waitTimeoutInMs_;
        DWORD               filter_;
    };

} /*vfs*/
