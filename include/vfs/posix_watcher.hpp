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
        posix_watcher(const path &dir, const callback_t &callback)
            : running_(false)
            , dir_(dir)
            , callback_(callback)
            , notifyInstance_(-1)
        {}

        //------------------------------------------------------------------------------------------
        posix_watcher(posix_watcher &&other)
            : running_(other.running_.load())
            , dir_(std::move(other.dir_))
            , callback_(std::move(other.callback_))
            , thread_(std::move(other.thread_))
            , notifyInstance_(other.notifyInstance_)
        {
            // ISSUE:
            // No move constructor for std::mutex or std::condition_variable.
        }

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
            
            notifyInstance_ = inotify_init();

            if (notifyInstance_ == -1)
            {
                vfs_errorf("ionotify_init() failed with error: %s", get_last_error_as_string(errno).c_str());
                return false;
            }

            auto mask = IN_CREATE | IN_DELETE | IN_MOVED_TO;
            
            const auto watchDescriptor = inotify_add_watch(notifyInstance_, dir_.str().c_str(), mask);

            if (watchDescriptor == -1)
            {
                vfs_errorf("inotify_add_watch() failed with error: %s", get_last_error_as_string(errno).c_str());
                return false;
            }


            running_ = true;

            thread_ = std::thread(
            [this, folders, files, watchDescriptor]
            {
                // make another thread
                auto watchingThread = std::thread([this, folders, files]
                {
                     run(folders, files);
                });
                    
                std::unique_lock<std::mutex> lk(eventMutex_);
                event_.wait(lk);
                // Removing a watch causes an IN_IGNORED event to be generated for this watchDescriptor.
                const auto error = inotify_rm_watch(notifyInstance_, watchDescriptor);

                if (error == -1)
                {
                    vfs_errorf("inotify_rm_watch() failed with error: %s", get_last_error_as_string(errno).c_str());
                    // Compile and link with -pthread
                    pthread_kill(thread_.native_handle(), SIGINT);
                }
                watchingThread.join();
            });

            return true;
        }

        //------------------------------------------------------------------------------------------
        bool stopWatching()
        {
            running_ = false;
            event_.notify_one();
            return true;
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
            // Properly close when stop signal is triggered.
            /*signal(SIGINT, [this](int)
            {
                vfs_check(running_ == false);
            });*/
            
            // Call the callback in case we have some folders in there waiting before we started the process
            callback_(dir_);

            while (running_)
            {
                struct inotify_event ev;

                // Will block until a new event takes place.
                const auto size = read(notifyInstance_, &ev, sizeof(inotify_event));

                if ((ev.mask & IN_IGNORED) != 0)
                {
                    // Caller thread wants to end program.
                    vfs_check(running_ == false);
                    return;
                }
                else if (strlen(ev.name) == 0)
                {
                    // WARNING: I'm not sure if ev.name is initialized with an empty string.
                    // Then a folder has been created or deleted.
                    if (folders)
                    {
                        callback_(dir_);
                    }
                }
                else
                {
                    // Then a file has been created or deleted
                    if (files)
                    {
                        callback_(dir_);
                    }
                }
            }
        }

    private:
        //------------------------------------------------------------------------------------------
        std::atomic<bool>           running_;
        path                        dir_;
        int32_t                     notifyInstance_;
        callback_t                  callback_;
        std::thread                 thread_;
        std::condition_variable     event_;
        std::mutex                  eventMutex_;
    };

} /*vfs*/
