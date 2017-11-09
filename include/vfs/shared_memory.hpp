#pragma once

#include "vfs/file_view.hpp"


namespace vfs {

    //----------------------------------------------------------------------------------------------
    using shared_memory_stream  = file_view_stream;
    //----------------------------------------------------------------------------------------------
    using shared_memory_sptr    = std::shared_ptr<shared_memory_stream>;
    using shared_memory_wptr    = std::weak_ptr<shared_memory_stream>;
    //----------------------------------------------------------------------------------------------

    //----------------------------------------------------------------------------------------------
    inline auto create_shared_memory(const path &name, int64_t size)
    {
        return shared_memory_sptr(new shared_memory_stream(name, size, false));
    }
    //----------------------------------------------------------------------------------------------

    //----------------------------------------------------------------------------------------------
    inline auto open_shared_memory(const path &name, int64_t viewSize = 0)
    {
        return shared_memory_sptr(new shared_memory_stream(name, viewSize, true));
    }
    //----------------------------------------------------------------------------------------------

} /*vfs*/
