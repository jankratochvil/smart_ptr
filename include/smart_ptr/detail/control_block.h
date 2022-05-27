//
// This file is part of smart_ptr project <https://github.com/romanpauk/smart_ptr>
//
// See LICENSE for license and copyright information
// SPDX-License-Identifier: AGPL-3.0-or-later
//

#pragma once

#include <memory>

namespace smart_ptr
{
    template < typename T > struct default_deleter
    {
        void operator()(T* ptr)
        {
            delete ptr;
        }
    };

    template < typename T, typename Counter > class control_block_base
    {
    public:
        template < typename... Args > control_block_base(T* p, Args&&... args)
            : counter(std::forward< Args >(args)...)
            , ptr(p)
        {}

        virtual ~control_block_base() {}
        virtual void deallocate() = 0;

        Counter counter;
        T* ptr;
    };

    template < typename Allocator > class control_block_allocator
    {
    public:
        template < typename AllocatorT > control_block_allocator(AllocatorT&& allocator)
            : allocator_(std::forward< AllocatorT >(allocator))
        {}

        Allocator& get_allocator() { return allocator_; }

    private:
        Allocator allocator_;
    };

    template < typename T > class control_block_allocator< std::allocator< T > >
    {
    public:
        template < typename AllocatorT > control_block_allocator(AllocatorT&& allocator) {}
        std::allocator< T > get_allocator() { return std::allocator< T >(); }
    };

    template < typename Deleter > class control_block_deleter
    {
    public:
        template < typename DeleterT > control_block_deleter(DeleterT&& deleter)
            : deleter_(std::forward< DeleterT >(deleter))
        {}

        Deleter& get_deleter() { return deleter_; }

    private:
        Deleter deleter_;
    };

    template < typename T > class control_block_deleter< default_deleter< T > >
    {
    public:
        template < typename DeleterT > control_block_deleter(DeleterT&&) {}
        default_deleter< T > get_deleter() { return default_deleter< T >(); }
    };

    template < typename T, typename Counter, typename Allocator, typename Deleter > class control_block
        : public control_block_base< T, Counter >
        , public control_block_allocator< Allocator >
        , public control_block_deleter< Deleter >
    {
        using allocator_type = typename std::allocator_traits< Allocator >::template rebind_alloc<
            control_block< T, Counter, Allocator, Deleter >
        >;

    public:
        template < typename AllocatorT, typename DeleterT, typename... Args >
        control_block(AllocatorT&& allocator, DeleterT&& deleter, T* p, Args&&... args)
            : control_block_base< T, Counter >(p, std::forward< Args >(args)...)
            , control_block_allocator< Allocator >(std::forward< AllocatorT >(allocator))
            , control_block_deleter< Deleter >(std::forward< DeleterT >(deleter))
        {}

        ~control_block()
        {
            get_deleter()(ptr);
        }

        template < typename AllocatorT, typename DeleterT, typename... Args >
        static control_block< T, Counter, Allocator, Deleter >* allocate(AllocatorT&& allocator, DeleterT&& deleter, Args&&... args)
        {
            allocator_type alloc(allocator);
            auto ptr = std::allocator_traits< allocator_type >::template allocate(alloc, 1);
            std::allocator_traits< allocator_type >::template construct(
                alloc, ptr, std::forward< AllocatorT >(allocator), std::forward< DeleterT >(deleter), std::forward< Args >(args)...);
            return ptr;
        }

        void deallocate() override
        {
            allocator_type alloc(get_allocator());
            std::allocator_traits< allocator_type >::template destroy(alloc, this);
            std::allocator_traits< allocator_type >::template deallocate(alloc, this, 1);
        }
    };
}
