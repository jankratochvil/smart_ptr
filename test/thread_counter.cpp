//
// This file is part of smart_ptr project <https://github.com/romanpauk/smart_ptr>
//
// See LICENSE for license and copyright information
// SPDX-License-Identifier: AGPL-3.0-or-later
//

#include <smart_ptr/shared_ptr.h>
#include <smart_ptr/detail/thread_counter.h>

#include <gtest/gtest.h>

#if defined(_WIN32)
#include <intrin.h>
#endif

TEST(thread_counter, zero_index_sse4)
{
    size_t count = 0;
    std::array< uint32_t, 16 > values = {};

    count = smart_ptr::zero_index_sse4(values);
    EXPECT_EQ(count, 0);

    values[0] = 1;
    count = smart_ptr::zero_index_sse4(values);
    EXPECT_EQ(count, 1);

    memset(values.data(), 1, sizeof(values));
    count = smart_ptr::zero_index_sse4(values);
    EXPECT_EQ(count, values.size());
}
