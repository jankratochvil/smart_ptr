//
// This file is part of smart_ptr project <https://github.com/romanpauk/smart_ptr>
//
// See LICENSE for license and copyright information
// SPDX-License-Identifier: AGPL-3.0-or-later
//

#include <smart_ptr/shared_ptr.h>
#include <smart_ptr/detail/shared_counter.h>
#include <smart_ptr/detail/biased_counter.h>

#include <gtest/gtest.h>
#include <memory>

using shared_ptr_types = ::testing::Types<
    std::shared_ptr< int >,
    smart_ptr::shared_ptr< int, smart_ptr::shared_counter< uint64_t, false > >,
    smart_ptr::shared_ptr< int, smart_ptr::shared_counter< uint64_t, true > >,
    smart_ptr::shared_ptr< int, smart_ptr::biased_counter< uint64_t > >
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
    smart_ptr::make_shared< int, smart_ptr::shared_counter< uint32_t, true > >(1);
}
