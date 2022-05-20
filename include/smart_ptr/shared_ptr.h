//
// This file is part of smart_ptr project <https://github.com/romanpauk/smart_ptr>
//
// See LICENSE for license and copyright information
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Based on the article "Biased reference counting: minimizing atomic operations in garbage collection"
// available at: https://dl.acm.org/doi/pdf/10.1145/3243176.3243195
//

namespace smart_ptr
{
    template < typename T, typename Counter > class shared_ptr
    {
    public:
        using element_type = T;

        shared_ptr() = default;
        
        explicit shared_ptr(T* p)
            : counter_(new Counter(1))
            , p_(p)
        {}

        shared_ptr(const shared_ptr< T, Counter >& other)
            : counter_(other.counter_)
            , p_(other.p_)
        {
            increment();
        }
        
        ~shared_ptr()
        {
            decrement();
        }

        shared_ptr< T, Counter >& operator = (const shared_ptr< T, Counter >& other)
        {
            decrement();

            counter_ = other.counter_;
            p_ = other.p_;
            increment();

            return *this;
        }

        T* operator ->()
        {
            assert(p_);
            return p_;
        }

        const T* operator ->() const
        {
            assert(p_);
            return p_;
        }

        T& operator *()
        {
            assert(p_);
            return *p_;
        }

        const T& operator *() const
        {
            assert(p_);
            return *p_;
        }

    private:
        void increment()
        {
            if(counter_)
            {
                counter_->increment();
            }
        }

        void decrement()
        {
            if (counter_ && counter_->decrement())
            {
                delete counter_;
                delete p_;
            }
        }

        T* p_{};
        Counter* counter_{};
    };
}
