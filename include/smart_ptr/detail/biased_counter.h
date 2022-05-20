//
// This file is part of smart_ptr project <https://github.com/romanpauk/smart_ptr>
//
// See LICENSE for license and copyright information
// SPDX-License-Identifier: AGPL-3.0-or-later
//

#include <smart_ptr/detail/thread_traits.h>

namespace smart_ptr
{
    template < typename T, typename ThreadTraits = default_thread_traits > struct biased_counter
    {
        biased_counter(T ref)
            : tid_(ThreadTraits::get_current_thread_id())
            , refs_(ref)
            , refs_local_(ref)
        {}

        void increment()
        {
            if (tid_ == ThreadTraits::get_current_thread_id())
            {
                ++refs_local_;
            }
            else
            {
                ++refs_;
            }
        }

        bool decrement()
        {
            if (tid_ == ThreadTraits::get_current_thread_id())
            {
                if (--refs_local_ == 0)
                {
                    return --refs_ == 0;
                }
            }
            else
            {
                return --refs_ == 0;
            }

            return false;
        }

    private:
        T refs_local_;
        typename ThreadTraits::thread_id tid_;
        std::atomic< T > refs_;
    };
}
