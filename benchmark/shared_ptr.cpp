//
// This file is part of smart_ptr project <https://github.com/romanpauk/smart_ptr>
//
// See LICENSE for license and copyright information
// SPDX-License-Identifier: AGPL-3.0-or-later
//

#include <smart_ptr/shared_ptr.h>
#include <smart_ptr/detail/shared_counter.h>
#include <smart_ptr/detail/biased_counter.h>

#include <benchmark/benchmark.h>

BENCHMARK_MAIN();

template < typename T > static void shared_ptr_copy_ctor(benchmark::State& state)
{
    static thread_local T value(new typename T::element_type());

    for (auto _ : state)
    {
        T ptr(value);
    }
}

auto max_threads = std::thread::hardware_concurrency();

BENCHMARK_TEMPLATE(shared_ptr_copy_ctor, smart_ptr::shared_ptr< int, smart_ptr::shared_counter< uint64_t, false > >)->UseRealTime();
BENCHMARK_TEMPLATE(shared_ptr_copy_ctor, std::shared_ptr< int >)->ThreadRange(1, max_threads)->UseRealTime();
BENCHMARK_TEMPLATE(shared_ptr_copy_ctor, smart_ptr::shared_ptr< int, smart_ptr::biased_counter< uint64_t > >)->ThreadRange(1, max_threads)->UseRealTime();
BENCHMARK_TEMPLATE(shared_ptr_copy_ctor, smart_ptr::shared_ptr< int, smart_ptr::shared_counter< uint64_t, true > >)->ThreadRange(1, max_threads)->UseRealTime();
