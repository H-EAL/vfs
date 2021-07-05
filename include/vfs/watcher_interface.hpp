#pragma once

#include <chrono>
#include <functional>
#include "vfs/path.hpp"


namespace vfs {

    //----------------------------------------------------------------------------------------------
    template<typename _Impl>
    class watcher_interface
        : _Impl
    {
    public:
        //------------------------------------------------------------------------------------------
        using callback_t = std::function<void(const path&)>;

    public:
        //------------------------------------------------------------------------------------------
        using base_type = _Impl;
        using self_type = watcher_interface<_Impl>;

    public:
        //------------------------------------------------------------------------------------------
        template<typename R, typename P>
        watcher_interface(const path &dir, std::chrono::duration<R,P> waitTimeout, const callback_t &callback)
            : base_type(dir, waitTimeout, callback)
        {}

        //------------------------------------------------------------------------------------------
        watcher_interface(const path &dir, const callback_t &callback)
            : base_type(dir, callback)
        {}

        //------------------------------------------------------------------------------------------
        ~watcher_interface()
        {
            stopWatching();
            wait();
        }

    public:
        //------------------------------------------------------------------------------------------
        bool startWatching(bool folders, bool files)
        {
            return base_type::startWatching(folders, files);
        }
        //------------------------------------------------------------------------------------------
        bool stopWatching()
        {
            return base_type::stopWatching();
        }
        //------------------------------------------------------------------------------------------
        void wakeUp()
        {
            return base_type::wakeUp();
        }
        //------------------------------------------------------------------------------------------
        void wait()
        {
            return base_type::wait();
        }
    };
    //----------------------------------------------------------------------------------------------

} /*vfs*/
