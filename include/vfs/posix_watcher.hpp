#pragma once

#include <atomic>
#include <thread>
#include <filesystem>
#include <condition_variable>
#include <mutex>
#include <functional>
#include <sys/inotify.h>
#include <unistd.h>
#include <signal.h>

#include "vfs/platform.hpp"
#include "vfs/path.hpp"


namespace vfs {

    //----------------------------------------------------------------------------------------------
    using watcher_impl = class posix_watcher;


    //----------------------------------------------------------------------------------------------
    class posix_watcher
    {
        using callback_t = std::function<void(const path&)>;

    public:
        //------------------------------------------------------------------------------------------
        template<typename R, typename P>
        posix_watcher(const path &dir, std::chrono::duration<R, P> waitTimeout, const callback_t &callback)
            : running_(false)
            , dir_(dir)
            , callback_(callback)
            , inotifyFd_(-1)
            , waitTimeoutInMs_(std::chrono::duration_cast<std::chrono::milliseconds>(waitTimeout).count())
        {
            waitTimeoutInMs_ = (waitTimeoutInMs_ == 0) ? std::numeric_limits<uint64_t>::max() : waitTimeoutInMs_;
        }

        //------------------------------------------------------------------------------------------
        posix_watcher(const path &dir, const callback_t &callback)
            : running_(false)
            , dir_(dir)
            , callback_(callback)
            , inotifyFd_(-1)
            , waitTimeoutInMs_(std::numeric_limits<uint64_t>::max())
        {}

    public:
        //------------------------------------------------------------------------------------------
        posix_watcher(const posix_watcher &)                = delete;
        posix_watcher& operator =(const posix_watcher &)    = delete;

    public:
        //------------------------------------------------------------------------------------------
        bool startWatching(bool folders, bool files)
        {
            if (callback_ == nullptr)
            {
                vfs_errorf("NULL callback specified to watcher %s", dir_.c_str());
                return false;
            }

            inotifyFd_ = inotify_init();

            if (inotifyFd_ == -1)
            {
                vfs_errorf("ionotify_init() failed with error: %s", get_last_error_as_string(errno).c_str());
                return false;
            }

            auto mask = IN_CREATE | IN_DELETE | IN_MOVED_TO;

            const auto watchDescriptor = inotify_add_watch(inotifyFd_, dir_.c_str(), mask);
            if (watchDescriptor == -1)
            {
                vfs_errorf("inotify_add_watch() failed with error: %s", get_last_error_as_string(errno).c_str());
                return false;
            }

            running_ = true;

            thread_ = std::thread(
            [this, folders, files, watchDescriptor]
            {
                // Make another thread
                auto watchingThread = std::thread([this, folders, files]
                {
                    run(folders, files);
                });

                std::unique_lock<std::mutex> lk(cvMutex_);
                if(waitTimeoutInMs_ == std::numeric_limits<uint64_t>::max())
                {
                    cv_.wait(lk);
                }
                else
                {
                    while(running_)
                    {
                        const auto status = cv_.wait_for(lk, std::chrono::milliseconds(waitTimeoutInMs_));

                        const auto watcherStopped = status == std::cv_status::no_timeout && running_ == false;
                        if(watcherStopped)
                        {
                            break;
                        }

                        callback_(dir_);
                    }
                }

                // Removing a watch causes an IN_IGNORED event to be generated for this watchDescriptor.
                const auto error = inotify_rm_watch(inotifyFd_, watchDescriptor);
                if (error == -1)
                {
                    vfs_errorf("inotify_rm_watch() failed with error: %s", get_last_error_as_string(errno).c_str());
                    pthread_kill(thread_.native_handle(), SIGUSR1);
                }
                watchingThread.join();
            });

            return true;
        }

        //------------------------------------------------------------------------------------------
        bool stopWatching()
        {
            running_ = false;
            wakeUp();
            return true;
        }

        //------------------------------------------------------------------------------------------
        void wakeUp()
        {
            cv_.notify_one();
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
        void run(bool folders, bool files)
        {
            // This is set up to end the thread if killed by SIGUSR1.
            struct sigaction sa;
            sa.sa_handler   = nullptr;
            sa.sa_sigaction = nullptr;
            sigemptyset(&sa.sa_mask);
            sigaction(SIGUSR1, &sa, nullptr);

            // Call the callback in case we have some folders in there waiting before we started the process
            callback_(dir_);

            constexpr auto maxNumEventsToRead   = 128;
            constexpr auto eventSize            = sizeof(struct inotify_event);
            constexpr auto bufferLength         = maxNumEventsToRead*(eventSize+16);
            auto eventBuffer                    = std::vector<uint8_t>(bufferLength);

            while (running_)
            {
                // Multiple events can be read at a time.
                auto eventBufferOffset = 0;

                // Will block until a new event takes place.
                const auto sizeReadInBytes = read(inotifyFd_, eventBuffer.data(), bufferLength);
                if(sizeReadInBytes == -1)
                {
                    vfs_errorf("read() of inotify file descriptor failed with error: %s",
                               get_last_error_as_string(errno).c_str());
                    return;
                }

                while (eventBufferOffset < sizeReadInBytes)
                {
                    const auto pEvent = reinterpret_cast<struct inotify_event *>(&eventBuffer[eventBufferOffset]);

                    if ((pEvent->mask & IN_IGNORED) != 0)
                    {
                        // IN_IGNORED is set when the caller thread removes watch.
                        // This can also happen when the watched file/directory was deleted
                        // or the filesystem was unmounted.
                        if(running_ == false)
                        {
                            return;
                        }
                    }
                    else if((pEvent->mask & IN_ISDIR) != 0)
                    {
                        if (folders)
                        {
                            // A directory has been created/deleted.
                            callback_(dir_);
                        }
                    }
                    else if (pEvent->len != 0)
                    {
                        if (files)
                        {
                            // A file has been created/deleted.
                            callback_(dir_);
                        }
                    }

                    eventBufferOffset += eventSize + pEvent->len;
                }
            }
        }

    private:
        //------------------------------------------------------------------------------------------
        std::atomic<bool>           running_;
        path                        dir_;
        int32_t                     inotifyFd_;
        callback_t                  callback_;
        std::thread                 thread_;
        std::condition_variable     cv_;
        std::mutex                  cvMutex_;
        uint64_t                    waitTimeoutInMs_;
    };

} /*vfs*/
