#pragma once


namespace vfs {

    //----------------------------------------------------------------------------------------------
    using virtual_allocator_impl = struct win_virtual_allocator;

    //----------------------------------------------------------------------------------------------
    struct win_virtual_allocator
    {
        //------------------------------------------------------------------------------------------
        static void* reserve(int64_t sizeInByte)
        {
            return VirtualAlloc(nullptr, sizeInByte, MEM_RESERVE, PAGE_READWRITE);
        }
        //------------------------------------------------------------------------------------------
        static void* commit(void *pAddr, int64_t size)
        {
            return VirtualAlloc(pAddr, size, MEM_COMMIT, PAGE_READWRITE);
        }
        //------------------------------------------------------------------------------------------
        static bool deallocate(void *pAddr)
        {
            return VirtualFree(pAddr, 0, MEM_RELEASE) == TRUE;
        }
    };
    
} /*vfs*/
