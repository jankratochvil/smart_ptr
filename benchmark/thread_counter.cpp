//
// This file is part of smart_ptr project <https://github.com/romanpauk/smart_ptr>
//
// See LICENSE for license and copyright information
// SPDX-License-Identifier: AGPL-3.0-or-later
//

#include <smart_ptr/shared_ptr.h>
#include <smart_ptr/detail/thread_counter.h>

#include <benchmark/benchmark.h>

static void zero_index(benchmark::State& state)
{
    alignas(32) std::array< uint32_t, 16 > values = { 1 };

    for (auto _ : state)
    {
        volatile size_t index = smart_ptr::zero_index(values);
    }
}

BENCHMARK(zero_index)->UseRealTime();

static void zero_index_sse4(benchmark::State& state)
{
    alignas(32) std::array< uint32_t, 16 > values = { 1 };

    for (auto _ : state)
    {
        volatile size_t index = smart_ptr::zero_index_sse4(values);
    }
}

BENCHMARK(zero_index_sse4)->UseRealTime();
