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

TEST(shared_ptr_test, std)
{
    std::shared_ptr< int > p1(new int(1));
    std::shared_ptr< int > p2 = p1;
}

TEST(shared_ptr_test, shared_counter)
{
    smart_ptr::shared_ptr< int, smart_ptr::shared_counter< uint64_t, true > > p1(new int(1));
    smart_ptr::shared_ptr< int, smart_ptr::shared_counter< uint64_t, true > > p2(p1);
    smart_ptr::shared_ptr< int, smart_ptr::shared_counter< uint64_t, true > > p3 = p1;
}

TEST(shared_ptr_test, biased_counter)
{
    smart_ptr::shared_ptr< int, smart_ptr::biased_counter< uint64_t > > p1(new int(1));
    smart_ptr::shared_ptr< int, smart_ptr::biased_counter< uint64_t >> p2(p1);
    smart_ptr::shared_ptr< int, smart_ptr::biased_counter< uint64_t > > p3 = p1;
}
