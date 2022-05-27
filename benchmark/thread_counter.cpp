//
// This file is part of smart_ptr project <https://github.com/romanpauk/smart_ptr>
//
// See LICENSE for license and copyright information
// SPDX-License-Identifier: AGPL-3.0-or-later
//

#include <smart_ptr/shared_ptr.h>
#include <smart_ptr/detail/thread_counter.h>
#include <smart_ptr/detail/biased_counter.h>

#include <benchmark/benchmark.h>
#include <thread>

#include <smart_ptr/detail/cpu_traits.h>

static const auto max_threads = std::thread::hardware_concurrency();

static void find_index(benchmark::State& state)
{
    alignas(32) std::array< uint32_t, 16 > values = { 1 };

    for (auto _ : state)
    {
        volatile size_t index = smart_ptr::find_index(values, 0);
    }
}

BENCHMARK(find_index)->UseRealTime();

static void find_index_sse4(benchmark::State& state)
{
    alignas(32) std::array< uint32_t, 16 > values = { 1 };

    for (auto _ : state)
    {
        volatile size_t index = smart_ptr::find_index_sse4(values, 0);
    }
}

BENCHMARK(find_index_sse4)->UseRealTime();

static void increment_uint32_t(benchmark::State& state)
{
    static uint32_t value = 0;
    for (auto _ : state)
    {
        value++;
    }
    volatile uint32_t result = value;
}

BENCHMARK(increment_uint32_t)->UseRealTime()->ThreadRange(1, max_threads);

/*
static void increment_thread_local_uint32_t(benchmark::State& state)
{
    static thread_local uint32_t value = 0;
    for (auto _ : state)
    {
        value++;
    }
    volatile uint32_t result = value;
}

BENCHMARK(increment_thread_local_uint32_t)->UseRealTime()->ThreadRange(1, max_threads);
*/

static void increment_atomic(benchmark::State& state)
{
    static std::atomic< uint32_t > value = 0;
    for (auto _ : state)
    {
        value++;
    }
    volatile uint32_t result = value;
}

BENCHMARK(increment_atomic)->UseRealTime()->ThreadRange(1, max_threads);

static void increment_thread_local_atomic(benchmark::State& state)
{
    static thread_local std::atomic< uint32_t > value = 0;
    for (auto _ : state)
    {
        value++;
    }
    volatile uint32_t result = value;
}

BENCHMARK(increment_thread_local_atomic)->UseRealTime()->ThreadRange(1, max_threads);

static void increment_cpu_local_atomic_thread_index(benchmark::State& state)
{
    static std::array< smart_ptr::aligned< std::atomic< uint32_t >, 64 >, 32 > values;

    for (auto _ : state)
    {
        ++values[state.thread_index];
    }

    volatile uint32_t result = values[1];
}

BENCHMARK(increment_cpu_local_atomic_thread_index)->UseRealTime()->ThreadRange(1, max_threads);

static void increment_cpu_local_atomic_cpu_traits(benchmark::State& state)
{
    static std::array< smart_ptr::aligned< std::atomic< uint32_t >, 64 >, 32 > values;

    for (auto _ : state)
    {
        ++values[smart_ptr::cpu_traits_win32::get_current_cpu_id()];
    }

    volatile uint32_t result = values[1];
}

BENCHMARK(increment_cpu_local_atomic_cpu_traits)->UseRealTime()->ThreadRange(1, max_threads);
