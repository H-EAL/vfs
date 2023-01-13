#pragma once

#include <sys/mman.h>


namespace vfs {

    //----------------------------------------------------------------------------------------------
    using virtual_allocator_impl = struct posix_virtual_allocator;

    //----------------------------------------------------------------------------------------------
    struct posix_virtual_allocator
    {
        //------------------------------------------------------------------------------------------
        static void* reserve(int64_t size)
        {
            return mmap(nullptr, size, PROT_NONE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
        }
        //------------------------------------------------------------------------------------------
        static void* commit(void *pAddr, int64_t size)
        {
            return mprotect(pAddr, size, PROT_READ | PROT_WRITE) == 0 ? pAddr : nullptr;
        }
        //------------------------------------------------------------------------------------------
        static bool deallocate(void *pAddr)
        {
            return munmap(pAddr, 0);
        }
    };

} /*ftl*/
