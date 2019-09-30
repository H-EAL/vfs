#pragma once


namespace vfs {

    //----------------------------------------------------------------------------------------------
    template<typename _Impl>
    struct virtual_allocator_interface
        : public _Impl
    {
        //------------------------------------------------------------------------------------------
        template<typename T>
        static T* reserve(int64_t elementCount)
        {
            return reinterpret_cast<T*>(_Impl::reserve(elementCount * sizeof(T)));
        }

        //------------------------------------------------------------------------------------------
        static void* reserve(int64_t sizeInByte)
        {
            return _Impl::reserve(sizeInByte);
        }

        //------------------------------------------------------------------------------------------
        static void* commit(void *pAddr, int64_t size)
        {
            return _Impl::commit(pAddr, size);
        }

        //------------------------------------------------------------------------------------------
        static bool deallocate(void *pAddr)
        {
            return _Impl::deallocate(pAddr);
        }
    };
    
} /*vfs*/
