//
// This file is part of smart_ptr project <https://github.com/romanpauk/smart_ptr>
//
// See LICENSE for license and copyright information
// SPDX-License-Identifier: AGPL-3.0-or-later
//

#include <array>
#include <atomic>

#if defined(_WIN32)
#include <intrin.h>
#endif

namespace smart_ptr
{
    static size_t zero_index_sse4(const std::array< uint32_t, 16 >& values)
    {
        __m256i v = _mm256_loadu_si256((const __m256i*)values.data());
        __m256i vcmp = _mm256_cmpeq_epi32(v, _mm256_setzero_si256());
        unsigned bitmask = _mm256_movemask_ps(_mm256_castsi256_ps(vcmp));
        bitmask |= 1 << 16;
        return _tzcnt_u32(bitmask);
    }

    template< typename T, size_t N > size_t zero_index(const std::array< T, N >& values)
    {
        for (size_t i = 0; i < values.size(); ++i)
        {
            if (values[i] == 0)
            {
                return i;
            }
        }

        return values.size();
    }

    template < typename T, size_t N = 64/sizeof(T) > struct thread_counter
    {
        thread_counter(T ref)
            : refs_(ref)
        {            
            auto& local = get_refs_local();
            local_index_ = zero_index_sse4(local);
            if (local_index_ < local.size())
            {
                local[local_index_] = 1;
            }
        }

        void increment()
        {
            auto& local = get_refs_local();
            if (local_index_ < local.size())
            {
                ++local[local_index_];
            }
            else
            {
                ++refs_;
            }
        }

        bool decrement()
        {
            auto& local = get_refs_local();
            if (local_index_ < local.size())
            {
                if (--local[local_index_] == 0)
                {
                    if (--refs_ == 0)
                    {
                        return true;
                    }
                }
            }
            else
            {
                if (--refs_ == 0)
                {
                    return true;
                }
            }

            return false;
        }

    private:
        std::array< T, N >& get_refs_local()
        {
            alignas(32) static thread_local std::array< T, N > local;
            return local;
        }

        std::atomic< T > refs_ {};
        size_t local_index_;
    };
}
