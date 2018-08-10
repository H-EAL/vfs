#pragma once

#include <set>
#include <mutex>
#include <atomic>

#include "vfs/logging.hpp"


namespace vfs {
    
    //----------------------------------------------------------------------------------------------
    template<typename T, uint32_t _MaxElementCount>
    class virtual_array
    {
        static_assert(sizeof(T) >= sizeof(uint32_t),
            "T size must be >= 4 bytes so that the freelist can fit in.");

    private:
        //------------------------------------------------------------------------------------------
        static constexpr auto page_size             = 0x1000; // 4kB
        static constexpr auto element_size          = sizeof(T);
        static constexpr auto elements_per_page     = page_size / element_size;
        static constexpr auto invalid_index         = UINT32_MAX;
        static constexpr auto control_bits_per_page = page_size * 8;

    public:
        //------------------------------------------------------------------------------------------
        struct iterator
        {
            iterator(virtual_array<T, _MaxElementCount> &arr, uint32_t index)
                : arr_(arr)
                , currentIndex_(index)
            {
                goToNextValidIndex();
            }

            iterator& operator ++()
            {
                ++currentIndex_;
                goToNextValidIndex();
                return (*this);
            }

            bool operator ==(const iterator &rhs) const
            {
                return &arr_ == &(rhs.arr_) && currentIndex_ == rhs.currentIndex_;
            }

            bool operator !=(const iterator &rhs) const
            {
                return !(*this == rhs);
            }

        private:
            void goToNextValidIndex()
            {
                while (currentIndex_ < arr_.lastValidIndex_
                    && (arr_.pControlRegister_[currentIndex_ / 64] & (1ull << (currentIndex_ % 64))) == 0)
                {
                    ++currentIndex_;
                }
            }

        private:
            virtual_array<T, _MaxElementCount> &arr_;
            uint32_t                            currentIndex_;
        };

        //------------------------------------------------------------------------------------------
        struct const_iterator
        {
            const_iterator(const virtual_array<T, _MaxElementCount> &arr, uint32_t index)
                : arr_(arr)
                , currentIndex_(index)
            {
                goToNextValidIndex();
            }

            const_iterator& operator ++()
            {
                ++currentIndex_;
                goToNextValidIndex();
                return (*this);
            }

            bool operator ==(const const_iterator &rhs) const
            {
                return &arr_ == &(rhs.arr_) && currentIndex_ == rhs.currentIndex_;
            }

            bool operator !=(const const_iterator &rhs) const
            {
                return !(*this == rhs);
            }

            const T& operator *() const
            {
                return arr_[currentIndex_];
            }

        private:
            void goToNextValidIndex()
            {
                while (currentIndex_ < arr_.lastValidIndex_
                    && (arr_.pControlRegister_[currentIndex_ / 64] & (1ull << (currentIndex_ % 64))) == 0)
                {
                    ++currentIndex_;
                }
            }

        private:
            const virtual_array<T, _MaxElementCount> &arr_;
            uint32_t                            currentIndex_;
        };

    public:
        //------------------------------------------------------------------------------------------
        iterator begin()
        {
            return iterator(*this, 0);
        }

        //------------------------------------------------------------------------------------------
        iterator end()
        {
            return iterator(*this, lastValidIndex_);
        }

        //------------------------------------------------------------------------------------------
        const_iterator begin() const
        {
            return const_iterator(*this, 0);
        }

        //------------------------------------------------------------------------------------------
        const_iterator end() const
        {
            return const_iterator(*this, lastValidIndex_);
        }

    public:
        //------------------------------------------------------------------------------------------
        virtual_array()
        {
            reset();
            init();
        }

        //------------------------------------------------------------------------------------------
        ~virtual_array()
        {
            if constexpr (!std::is_trivially_destructible_v<T>)
            {
                for (auto i = 0u; i < lastValidIndex_; ++i)
                {
                    if (pControlRegister_[i/64] & (1ull << (i%64)))
                    {
                        pArray_[i].~T();
                    }
                }
            }
            
            deallocate();
            pArray_ = nullptr;
            pControlRegister_ = nullptr;
        }

        //------------------------------------------------------------------------------------------
        virtual_array(const virtual_array &other)               = delete;
        virtual_array& operator =(const virtual_array &other)   = delete;

        //------------------------------------------------------------------------------------------
        virtual_array(virtual_array &&other)
        {
            swap(other);
            other.reset();
        }

        //------------------------------------------------------------------------------------------
        virtual_array& operator =(virtual_array &&other)
        {
            if (this != &other)
            {
                swap(other);
                other.~virtual_array();
            }
            return (*this);
        }

    public:
        //------------------------------------------------------------------------------------------
        template<typename... _Args>
        uint32_t emplace(_Args &&...args)
        {
            auto freeIndex = grabNextFreeIndex();
            new(pArray_ + freeIndex) T(std::forward<_Args>(args)...);

            // Mark as used here after actually constructing the element so iteration remains correct.
            pControlRegister_[freeIndex / 64].fetch_or(1ull << (freeIndex % 64));

            ++size_;
            return freeIndex;
        }

        //------------------------------------------------------------------------------------------
        void remove(uint32_t index)
        {
            ///TODO@h: keep an eye on this.
            // Mark as unused here before actually deleting the element so iteration remains correct.
            // This part is a bit tricky as we might already be iterating over this element while
            // we are destroying it. Might cause some weird issues.
            pControlRegister_[index / 64].fetch_and(~(1ull << (index % 64)));

            if constexpr (!std::is_trivially_destructible_v<T>)
            {
                pArray_[index].~T();
            }
            pushFreeList(index);

            --size_;
        }

        //------------------------------------------------------------------------------------------
        uint32_t size() const
        {
            return size_.load(std::memory_order_acquire);
        }

        //------------------------------------------------------------------------------------------
        T& operator [](uint32_t i)
        {
            return pArray_[i];
        }

        //------------------------------------------------------------------------------------------
        const T& operator [](uint32_t i) const
        {
            return pArray_[i];
        }

    private:
        //------------------------------------------------------------------------------------------
        void* reserve(int64_t size)
        {
        #if FTL_PLATFORM_WIN
            return VirtualAlloc(nullptr, size, MEM_RESERVE, PAGE_READWRITE));
        #elif FTL_PLATFORM_POSIX
            return mmap(nullptr, size, PROT_NONE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
        #endif
        }
        
        //------------------------------------------------------------------------------------------
        void* commit(void *pAddr, int64_t size)
        {
        #if FTL_PLATFORM_WIN
            return VirtualAlloc(pAddr, size, MEM_COMMIT, PAGE_READWRITE);
        #elif FTL_PLATFORM_POSIX
            return mprotect(pAddr, size, PROT_READ | PROT_WRITE) == 0 ? pAddr : nullptr;
        #endif 
        }
        
        //------------------------------------------------------------------------------------------
        void deallocate()
        {
        #if FTL_PLATFORM_WIN
            VirtualFree(pArray_, 0, MEM_RELEASE);
            VirtualFree(pControlRegister_, 0, MEM_RELEASE);
        #elif FTL_PLATFORM_POSIX
            munmap(pArray_, 0);
            munmap(pControlRegister_, 0);
        #endif
        }
    
        //------------------------------------------------------------------------------------------
        void init()
        {
            pArray_ = reinterpret_cast<T*>(reserve(_MaxElementCount * sizeof(T)));
            vfs_check(pArray_ != nullptr);
            pControlRegister_ = reinterpret_cast<std::atomic<uint64_t>*>(reserve(_MaxElementCount / sizeof(uint64_t)));
            vfs_check(pControlRegister_ != nullptr);
            grow(1);
        }

        //------------------------------------------------------------------------------------------
        void reset()
        {
            size_               = 0;
            pArray_             = nullptr;
            pageCount_          = 0;
            nextFreeIndex_      = invalid_index;
            lastValidIndex_     = 0;
            pControlRegister_   = nullptr;
            controlRegisterPageCount_ = 0;
        }

        //------------------------------------------------------------------------------------------
        void swap(virtual_array &other)
        {
            std::swap(size_             , other.size_               );
            std::swap(pArray_           , other.pArray_             );
            std::swap(pageCount_        , other.pageCount_          );
            std::swap(nextFreeIndex_    , other.nextFreeIndex_      );
            std::swap(lastValidIndex_   , other.lastValidIndex_     );
            std::swap(pControlRegister_ , other.pControlRegister_   );
            std::swap(controlRegisterPageCount_ , other.controlRegisterPageCount_);
        }

        //------------------------------------------------------------------------------------------
        void grow(uint32_t pageCount)
        {
            auto pData = commit(reinterpret_cast<uint8_t*>(pArray_) + pageCount_ * page_size, pageCount * page_size);
            (void)pData;
            vfs_check(pData != nullptr);
            pageCount_ += pageCount;

            const auto neededControlRegisterPageCount = uint32_t(elements_per_page * pageCount_ / control_bits_per_page) + 1 - controlRegisterPageCount_;
            if (neededControlRegisterPageCount > 0)
            {
                auto pData = commit(reinterpret_cast<uint8_t*>(pControlRegister_) + controlRegisterPageCount_ * page_size, neededControlRegisterPageCount * page_size);
                (void)pData;
                vfs_check(pData != nullptr);
                controlRegisterPageCount_ += neededControlRegisterPageCount;
            }
        }

        //------------------------------------------------------------------------------------------
        uint32_t grabNextFreeIndex()
        {
            while (!isFreeListEmpty())
            {
                const auto freeIndex = tryPopFreeList();
                if (freeIndex != invalid_index)
                {
                    return freeIndex;
                }
            }

            const auto freeIndex = lastValidIndex_++;

            const auto pageOfFreeIndex = (freeIndex / elements_per_page) + 1;
            if (pageOfFreeIndex > pageCount_)
            {
                std::lock_guard<std::mutex> __l(mutex_);
                if (pageOfFreeIndex > pageCount_)
                {
                    grow(pageCount_);
                }
            }

            return freeIndex;
        }

        //------------------------------------------------------------------------------------------
        bool isFreeListEmpty() const
        {
            return nextFreeIndex_ == invalid_index;
        }

        //------------------------------------------------------------------------------------------
        void pushFreeList(uint32_t freeIndex)
        {
            auto expectedNextFreeIndex = nextFreeIndex_.load(std::memory_order_acquire);
            vfs_check(expectedNextFreeIndex < lastValidIndex_ || expectedNextFreeIndex == invalid_index);
            do 
            {
                *reinterpret_cast<uint32_t*>(&pArray_[freeIndex]) = expectedNextFreeIndex;
            } while (!nextFreeIndex_.compare_exchange_strong(expectedNextFreeIndex, freeIndex, std::memory_order_acq_rel));

            auto i = nextFreeIndex_.load();
            (void)i;
            vfs_check(i < lastValidIndex_ || i == invalid_index);
        }

        //------------------------------------------------------------------------------------------
        uint32_t tryPopFreeList()
        {
            auto freeIndex = invalid_index;

            auto expectedNextFreeIndex = nextFreeIndex_.load(std::memory_order_acquire);

            if (expectedNextFreeIndex != invalid_index)
            {
                vfs_check(expectedNextFreeIndex < lastValidIndex_);
                const auto nextNextFreeIndex = *reinterpret_cast<uint32_t*>(&pArray_[expectedNextFreeIndex]);

                vfs_check(nextNextFreeIndex < lastValidIndex_ || nextNextFreeIndex == invalid_index);
                if (nextFreeIndex_.compare_exchange_strong(expectedNextFreeIndex, nextNextFreeIndex, std::memory_order_acq_rel))
                {
                    freeIndex = expectedNextFreeIndex;
                }

                auto i = nextFreeIndex_.load();
                (void)i;
                vfs_check(i < lastValidIndex_ || i == invalid_index);
            }
            return freeIndex;
        }

    private:
        //------------------------------------------------------------------------------------------
        std::atomic<uint32_t>   lastValidIndex_;
        T                       *pArray_;
        std::atomic<uint64_t>   *pControlRegister_;
        std::mutex              mutex_;
        std::atomic<uint32_t>   size_;
        uint32_t                pageCount_;
        uint32_t                controlRegisterPageCount_;
        std::atomic<uint32_t>   nextFreeIndex_;
    };
    
} /*ftl*/
