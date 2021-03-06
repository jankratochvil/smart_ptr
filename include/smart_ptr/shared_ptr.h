//
// This file is part of smart_ptr project <https://github.com/romanpauk/smart_ptr>
//
// See LICENSE for license and copyright information
// SPDX-License-Identifier: AGPL-3.0-or-later
//

#pragma once

#include <smart_ptr/detail/control_block.h>

#include <cassert>

namespace smart_ptr
{
    template < typename T, typename Counter > class shared_ptr
    {
        template < typename U, typename Allocator, typename CounterU, typename... Args > friend shared_ptr< U, CounterU > allocate_shared(Allocator&&, Args&&...);

        shared_ptr(control_block< T, Counter, std::allocator< T >, default_destructor< T >, true >* cb)
            : cb_(cb)
        {}

    public:
        using element_type = T;

        constexpr shared_ptr() noexcept = default;
        constexpr shared_ptr(std::nullptr_t) noexcept {}

        template < typename Y > explicit shared_ptr(Y* ptr)
            : cb_(control_block< T, Counter, std::allocator< T >, default_deleter< T >, false >::template allocate(
                std::allocator< T >(), default_deleter< T >(), ptr))
        {}

        template< typename Y, class Deleter > shared_ptr(Y* ptr, Deleter&& deleter)
            : cb_(control_block< T, Counter, std::allocator< T >, Deleter, false >::template allocate(
                std::allocator< T >(), std::forward< Deleter >(deleter), ptr))
        {}

        template< typename Y, class Deleter, class Allocator > shared_ptr(Y* ptr, Deleter&& deleter, Allocator&& alloc)
            : cb_(control_block< T, Counter, Allocator, Deleter, false >::template allocate(
                std::forward< Allocator >(alloc), std::forward< Deleter >(deleter), ptr))
        {}

        shared_ptr(const shared_ptr< T, Counter >& other)
            : cb_(other.cb_)
        {
            increment();
        }

        shared_ptr(shared_ptr< T, Counter >&& other)
            : cb_(nullptr)
        {
            std::swap(cb_, other.cb_);
        }

        ~shared_ptr()
        {
            decrement();
        }

        shared_ptr< T, Counter >& operator = (const shared_ptr< T, Counter >& other)
        {
            decrement();
            cb_ = other.cb_;
            increment();
            return *this;
        }

        shared_ptr< T, Counter >& operator = (shared_ptr< T, Counter >&& other)
        {
            decrement();
            std::swap(cb_, other.cb_);
            return *this;
        }

        T* operator ->()
        {
            assert(cb_);
            return cb_->get_ptr();
        }

        const T* operator ->() const
        {
            assert(cb_);
            return cb_->get_ptr();
        }

        T& operator *()
        {
            assert(cb_);
            return cb_->get_ptr();
        }

        const T& operator *() const
        {
            assert(cb_);
            return cb_->get_ptr();
        }

        T* get()
        {
            assert(cb_);
            return cb_->get_ptr();
        }

        const T* get() const
        {
            assert(cb_);
            return cb_->get_ptr();
        }

    private:
        void increment()
        {
            if(cb_)
            {
                cb_->increment();
            }
        }

        void decrement()
        {
            if (cb_ && cb_->decrement())
            {
                cb_->deallocate();
                cb_ = nullptr;
            }
        }
        
        control_block_base< T, Counter >* cb_{};
    };
    
    template < typename T, typename Allocator, typename Counter, typename... Args >
    shared_ptr< T, Counter > allocate_shared(Allocator&& allocator, Args&&... args)
    {
        return shared_ptr< T, Counter >(control_block< T, Counter, Allocator, default_destructor< T >, true >::template allocate(
            std::forward< Allocator >(allocator), default_destructor< T >(), std::forward< Args >(args)...
        ));
    }

    template < typename T, typename Counter, typename... Args >
    shared_ptr< T, Counter > make_shared(Args&&... args)
    {
        return allocate_shared< T, std::allocator< T >, Counter >(std::allocator< T >(), std::forward< Args >(args)...);
    }
}
