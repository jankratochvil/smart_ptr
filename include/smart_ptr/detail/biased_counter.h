//
// This file is part of smart_ptr project <https://github.com/romanpauk/smart_ptr>
//
// See LICENSE for license and copyright information
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Based on the article "Biased reference counting: minimizing atomic operations in garbage collection"
// available at: https://dl.acm.org/doi/pdf/10.1145/3243176.3243195
//

#pragma once

#include <smart_ptr/detail/thread_traits.h>

#include <array>

namespace smart_ptr
{
    template < typename T, typename ThreadTraits = default_thread_traits > struct biased_counter
    {
        biased_counter(void*)
            : tid_(ThreadTraits::get_current_thread_id())
            , refs_global_(1)
            , refs_local_(1)
        {}

        void increment(void*)
        {
            if (tid_ == ThreadTraits::get_current_thread_id())
            {
                ++refs_local_;
            }
            else
            {
                ++refs_global_;
            }
        }

        bool decrement(void*)
        {
            if (tid_ == ThreadTraits::get_current_thread_id())
            {
                if (--refs_local_ == 0)
                {
                    return --refs_global_ == 0;
                }
            }
            else
            {
                return --refs_global_ == 0;
            }

            return false;
        }

    private:
        T refs_local_;
        typename ThreadTraits::thread_id tid_;
        std::atomic< T > refs_global_;
    };
}
