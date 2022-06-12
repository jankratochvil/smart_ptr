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

#include <gtest/gtest.h>
#include <memory>

using shared_ptr_types = ::testing::Types<
    std::shared_ptr< int >
    , smart_ptr::shared_ptr< int, smart_ptr::shared_counter< uint64_t, false > >
    , smart_ptr::shared_ptr< int, smart_ptr::shared_counter< uint64_t, true > >
    , smart_ptr::shared_ptr< int, smart_ptr::biased_counter< uint64_t > >
    , smart_ptr::shared_ptr< int, smart_ptr::thread_counter< uint64_t, smart_ptr::thread_cache< uintptr_t, uint64_t, 8 > > >
>;

template <typename T> struct shared_ptr_test: public testing::Test {};
TYPED_TEST_SUITE(shared_ptr_test, shared_ptr_types);

TYPED_TEST(shared_ptr_test, ctor)
{
    TypeParam p1(new int(1));
    TypeParam p2(p1);
    TypeParam p3 = p1;

    TypeParam p4(new int(1), [](int* ptr) { delete ptr; });   
}

TEST(shared_ptr_test, make_shared)
{
    smart_ptr::make_shared< int, smart_ptr::shared_counter< uint64_t, true > >(1);
}
