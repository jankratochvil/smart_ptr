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

static const auto max_threads = std::thread::hardware_concurrency();

template < typename T > static void shared_ptr_copy_ctor(benchmark::State& state)
{
    static T value1(new typename T::element_type());
    static T value2(new typename T::element_type());
    static T value3(new typename T::element_type());
    static T value4(new typename T::element_type());
    static T value5(new typename T::element_type());
    static T value6(new typename T::element_type());
    static T value7(new typename T::element_type());
    static T value8(new typename T::element_type());
    
    for (auto _ : state)
    {
        T ptr1(value1);
        T ptr2(value2);
        T ptr3(value3);
        T ptr4(value4);
        T ptr5(value5);
        T ptr6(value6);
        T ptr7(value7);
        T ptr8(value8);
        T ptr9(value1);
        T ptr10(value2);
        T ptr11(value3);
        T ptr12(value4);
        T ptr13(value5);
        T ptr14(value6);
        T ptr15(value7);
        T ptr16(value8);
    }
}

BENCHMARK_TEMPLATE(shared_ptr_copy_ctor, smart_ptr::shared_ptr< int, smart_ptr::shared_counter< uint64_t, false > >)->UseRealTime();
BENCHMARK_TEMPLATE(shared_ptr_copy_ctor, std::shared_ptr< int >)->ThreadRange(1, max_threads)->MeasureProcessCPUTime()->UseRealTime();
BENCHMARK_TEMPLATE(shared_ptr_copy_ctor, smart_ptr::shared_ptr< int, smart_ptr::biased_counter< uint64_t > >)->ThreadRange(1, max_threads)->MeasureProcessCPUTime()->UseRealTime();
BENCHMARK_TEMPLATE(shared_ptr_copy_ctor, smart_ptr::shared_ptr< int, smart_ptr::shared_counter< uint64_t, true > >)->ThreadRange(1, max_threads)->UseRealTime();

