//
// This file is part of smart_ptr project <https://github.com/romanpauk/smart_ptr>
//
// See LICENSE for license and copyright information
// SPDX-License-Identifier: AGPL-3.0-or-later
//

#pragma once

#include <array>

#if defined(_WIN32)
#include <intrin.h>
#endif

namespace smart_ptr
{

    template< typename T, size_t N >  size_t find_index(const std::array< T, N >& values, T value)
    {
        for (size_t i = 0; i < values.size(); ++i)
        {
            if (values[i] == value)
                return i;
        }

        return values.size();
    }

    /*
    inline size_t find_index_sse4(const std::array< uint32_t, 16 >& values, uint32_t value)
    {
        __m256i v = _mm256_loadu_si256((const __m256i*)values.data());
        __m256i vcmp = _mm256_cmpeq_epi32(v, _mm256_set1_epi32(value));
        unsigned bitmask = _mm256_movemask_ps(_mm256_castsi256_ps(vcmp));
        bitmask |= 1 << 16;
        return _tzcnt_u32(bitmask);
    }
    */

    inline size_t find_index(const std::array< uint64_t, 8 >& values, uint64_t value)
    {
        __m256i v = _mm256_loadu_si256((const __m256i*)values.data());
        __m256i vcmp = _mm256_cmpeq_epi64(v, _mm256_set1_epi64x(value));
        unsigned bitmask = _mm256_movemask_ps(_mm256_castsi256_ps(vcmp));
        bitmask |= 1 << 16;
        return _tzcnt_u32(bitmask);
    }
/*
    inline size_t find_index_sse4(const std::array< uint64_t, 16 >& values, uint64_t value)
    {
        __m256i v = _mm256_loadu_si256((const __m256i*)values.data());
        __m256i vcmp = _mm256_cmpeq_epi64(v, _mm256_set1_epi64x(value));
        unsigned bitmask = _mm256_movemask_ps(_mm256_castsi256_ps(vcmp));
        bitmask |= 1 << 16;
        bitmask = _tzcnt_u32(bitmask);
        if(_tzcnt_u32(bitmask) < 16)
            return bitmask;

        v = _mm256_loadu_si256((const __m256i*)values.data() + 8 * sizeof(uint64_t));
        vcmp = _mm256_cmpeq_epi64(v, _mm256_set1_epi64x(value));
        bitmask = _mm256_movemask_ps(_mm256_castsi256_ps(vcmp));
        bitmask |= 1 << 16;
        return _tzcnt_u32(bitmask);
    }
 */

    template < typename Key, typename Value, size_t N > class thread_cache
    {
        static_assert(sizeof(Key) <= sizeof(uint64_t));
        static_assert(sizeof(Value) <= sizeof(uint64_t));

    public:
        size_t get(Key key) const
        {            
            auto index = find_index(get_local_keys(), key);
            if (index < N)
                return index;

            return find_index(get_local_keys(), Key());
        }

        void erase(size_t index)
        {
            assert(index < N);
            get_local_keys()[index] = 0;
        }

        size_t end() const
        {
            return N;
        }

        Value& operator [](size_t index)
        {
            assert(index < N);
            return get_local_values()[index];
        }

    private:
        static std::array< Key, N >& get_local_keys()
        {
            alignas(32) static thread_local std::array< Key, N > data;
            return data;
        }

        static std::array< Value, N >& get_local_values()
        {
            alignas(32) static thread_local std::array< Value, N > data;
            return data;
        }
    };

    template < typename Key, typename Value, size_t N > class thread_cache2
    {
        static_assert(sizeof(Key) <= sizeof(uint64_t));
        static_assert(sizeof(Value) <= sizeof(uint64_t));
        
    public:
        size_t get(Key key) const
        {
            auto index = load(key);
            if (index < N)
            {
                return index;
            }

            index = find_index(get_local_keys(), key);
            if (index >= N)
            {
                index = find_index(get_local_keys(), Key());
            }

            store(key, index);
            return index;
        }

        void erase(size_t index)
        {
            assert(index < N);
            get_local_keys()[index] = 0;
            invalidate(index);
        }

        size_t end() const
        {
            return N;
        }

        Value& operator [](size_t index)
        {
            assert(index < N);
            return get_local_values()[index];
        }

    private:
        static std::array< Key, N >& get_local_keys()
        {
            alignas(32) static thread_local std::array< Key, N > data;
            return data;
        }

        static std::array< Value, N >& get_local_values()
        {
            alignas(32) static thread_local std::array< Value, N > data;
            return data;
        }

        std::pair< Key, size_t >& cache() const
        {
            static thread_local std::pair< Key, size_t > cached;
            return cached;
        }

        size_t load(Key key) const
        {
            auto& pair = cache();
            return pair.first == key ? pair.second : N;
        }

        void store(Key key, size_t index) const
        {
            cache() = std::make_pair(key, index);
        }

        void invalidate(size_t index) const
        {
            auto& pair = cache();
            if(pair.second == index)
                pair.second = N;
        }
    };
}
