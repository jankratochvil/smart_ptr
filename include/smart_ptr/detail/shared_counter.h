//
// This file is part of smart_ptr project <https://github.com/romanpauk/smart_ptr>
//
// See LICENSE for license and copyright information
// SPDX-License-Identifier: AGPL-3.0-or-later
//

#include <atomic>

namespace smart_ptr
{
    template < typename T, bool IsAtomic > struct shared_counter;

    template < typename T > struct shared_counter< T, true >
    {
        shared_counter(T ref)
            : refs_(ref)
        {}

        void increment()
        {
            ++refs_;
        }

        bool decrement()
        {
            return --refs_ == 0;
        }

    private:
        std::atomic< T > refs_;
    };

    template < typename T > struct shared_counter< T, false >
    {
        shared_counter(T ref)
            : refs_(ref)
        {}

        void increment()
        {
            ++refs_;
        }

        bool decrement()
        {
            return --refs_ == 0;
        }

    private:
        T refs_;
    };
}