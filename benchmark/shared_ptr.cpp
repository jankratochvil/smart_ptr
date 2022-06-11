//
// This file is part of smart_ptr project <https://github.com/romanpauk/smart_ptr>
//
// See LICENSE for license and copyright information
// SPDX-License-Identifier: AGPL-3.0-or-later
//

#if defined(_WIN32)
    // TODO: move to cmake
    #define _ENABLE_EXTENDED_ALIGNED_STORAGE // Specifically enable standard behavior
#endif

#include <smart_ptr/shared_ptr.h>
#include <smart_ptr/detail/shared_counter.h>
#include <smart_ptr/detail/biased_counter.h>
#include <smart_ptr/detail/thread_counter.h>
#include <smart_ptr/detail/thread_cache.h>

#include <benchmark/benchmark.h>

static const auto max_threads = std::thread::hardware_concurrency();
static const auto min_ptrs = 1;
static const auto max_ptrs = 8;

template < typename T > static void copy_ctor(benchmark::State& state)
{
    const int values_size = 8;
    static std::array< T, 8 > values = []()
    {
        std::array < T, 8 > values;
        for(size_t i = 0; i < values.size(); ++i)
            values[i] = T(new typename T::element_type());
        return values;
    }();

    for (auto _ : state)
    {
        for (auto i = 0; i < state.range(0); ++i)
        {
	    volatile T tmp = values[i & (values.size())];
        }
    }
    state.SetBytesProcessed(state.iterations());
}

using shared_ptr = std::shared_ptr< int >;
using shared_ptr_shared_counter_st = smart_ptr::shared_ptr< int, smart_ptr::shared_counter< uint64_t, false > >;
using shared_ptr_shared_counter_mt = smart_ptr::shared_ptr< int, smart_ptr::shared_counter< uint64_t, true > >;
using shared_ptr_biased_counter = smart_ptr::shared_ptr< int, smart_ptr::biased_counter< uint64_t > >;
using shared_ptr_thread_counter_1 = smart_ptr::shared_ptr< int, smart_ptr::thread_counter< uint64_t, smart_ptr::thread_cache< uintptr_t, uint64_t, 8 > > >;
using shared_ptr_thread_counter_2 = smart_ptr::shared_ptr< int, smart_ptr::thread_counter< uint64_t, smart_ptr::thread_cache2< uintptr_t, uint64_t, 8 > > >;

BENCHMARK_TEMPLATE(copy_ctor, shared_ptr)->ThreadRange(1, max_threads)->UseRealTime()->Range(min_ptrs, max_ptrs);
//BENCHMARK_TEMPLATE(copy_ctor, shared_ptr_shared_counter_st)->UseRealTime()->Range(max_ptrs, max_ptrs);
//BENCHMARK_TEMPLATE(copy_ctor, shared_ptr_shared_counter_mt)->ThreadRange(1, max_threads)->UseRealTime()->Range(min_ptrs, max_ptrs);
//BENCHMARK_TEMPLATE(copy_ctor, shared_ptr_biased_counter)->ThreadRange(1, max_threads)->UseRealTime()->Range(min_ptrs, max_ptrs);
BENCHMARK_TEMPLATE(copy_ctor, shared_ptr_thread_counter_1)->ThreadRange(1, max_threads)->UseRealTime()->Range(min_ptrs, max_ptrs);
//BENCHMARK_TEMPLATE(copy_ctor, shared_ptr_thread_counter_2)->ThreadRange(1, max_threads)->UseRealTime()->Range(min_ptrs, max_ptrs);

BENCHMARK_MAIN();
