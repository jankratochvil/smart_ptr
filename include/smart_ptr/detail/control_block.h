//
// This file is part of smart_ptr project <https://github.com/romanpauk/smart_ptr>
//
// See LICENSE for license and copyright information
// SPDX-License-Identifier: AGPL-3.0-or-later
//

#pragma once

#include <memory>
#include <type_traits>

namespace smart_ptr
{
    template < typename T > struct default_deleter
    {
        void operator()(T* ptr)
        {
            delete ptr;
        }
    };

    template < typename T > struct default_destructor {};

    class control_block_dtor
    {
    public:
        virtual ~control_block_dtor() {}
        virtual void deallocate() = 0;
    };

    template < typename T, typename Counter > class control_block_base
        : public control_block_dtor
    {
    public:
        control_block_base()
            : counter_(this)
        {}

        control_block_base(T* ptr)
            : ptr_(p)
        {}

        void increment()
        {
            counter_.increment(this);
        }

        bool decrement()
        {
            return counter_.decrement(this);
        }       

        const T* get_ptr() const { return ptr_; }
              T* get_ptr()       { return ptr_; }

        void set_ptr(T* p) { ptr_ = p; }

    private:
        
        T* ptr_;
        Counter counter_;
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

    template < typename T > class control_block_deleter< default_destructor< T > >
    {
    public:
        template < typename DeleterT > control_block_deleter(DeleterT&&) {}
        default_destructor< T > get_deleter() { return default_destructor< T >(); }
    };

    template < typename Base, typename T, typename Allocator, typename Deleter, bool > class control_block_storage;

    template < typename Base, typename T, typename Allocator, typename Deleter > class control_block_storage< Base, T, Allocator, Deleter, true >
        : public control_block_allocator< typename std::allocator_traits< Allocator >::template rebind_alloc< T > >
    {
        using allocator_type = typename std::allocator_traits< Allocator >::template rebind_alloc< T >;

    public:
        template < typename AllocatorT, typename DeleterT, typename... Args > control_block_storage(
            AllocatorT&& allocator, DeleterT&&, Args&&... args
        )
            : control_block_allocator< allocator_type >(std::forward< AllocatorT >(allocator))
        {            
            std::allocator_traits< allocator_type >::construct(get_allocator(), get_ptr(), std::forward< Args >(args)...);
            static_cast<Base*>(this)->set_ptr(get_ptr());
        }

        ~control_block_storage()
        {            
            std::allocator_traits< allocator_type >::destroy(get_allocator(), get_ptr());
        }

    public:
        T* get_ptr() { return reinterpret_cast<T*>(&storage_); }

        std::aligned_storage_t< sizeof(T), alignof(T) > storage_;
    };

    template < typename Base, typename T, typename Allocator, typename Deleter > class control_block_storage< Base, T, Allocator, Deleter, false >
        : public control_block_allocator< Allocator >
        , public control_block_deleter< Deleter >
    {
    public:
        template < typename AllocatorT, typename DeleterT > control_block_storage(
            AllocatorT&& allocator, DeleterT&& deleter, T* p
        )
            : control_block_allocator< Allocator >(std::forward< AllocatorT >(allocator))
            , control_block_deleter< Deleter >(std::forward< DeleterT >(deleter))
        {
            static_cast<Base*>(this)->set_ptr(p);
        }

        ~control_block_storage()
        {
            get_deleter()(static_cast<Base*>(this)->get_ptr());
        }
    };

    template < typename T, typename Counter, typename Allocator, typename Deleter, bool Storage > class control_block
        : public control_block_base< T, Counter >
        , public control_block_storage< control_block< T, Counter, Allocator, Deleter, Storage >, T, Allocator, Deleter, Storage >
        
    {
        using allocator_type = typename std::allocator_traits< Allocator >::template rebind_alloc<
            control_block< T, Counter, Allocator, Deleter, Storage >
        >;

    public:
        template < typename AllocatorT, typename DeleterT, typename... Args >
        control_block(AllocatorT&& allocator, DeleterT&& deleter, Args&&... args)
            : control_block_storage< control_block< T, Counter, Allocator, Deleter, Storage >, T, Allocator, Deleter, Storage >(
                std::forward< AllocatorT >(allocator), std::forward< DeleterT >(deleter), std::forward< Args >(args)...
            )            
        {}

        template < typename AllocatorT, typename DeleterT >
        static control_block< T, Counter, Allocator, Deleter, false >* allocate(AllocatorT&& allocator, DeleterT&& deleter, T* ptr)
        {
            allocator_type alloc(allocator);
            auto cb = std::allocator_traits< allocator_type >::template allocate(alloc, 1);
            std::allocator_traits< allocator_type >::template construct(
                alloc, cb, std::forward< AllocatorT >(allocator), std::forward< DeleterT >(deleter), ptr);
            return cb;
        }

        template < typename AllocatorT, typename DeleterT, typename... Args >
        static control_block< T, Counter, Allocator, Deleter, true >* allocate(AllocatorT&& allocator, DeleterT&& deleter, Args&&... args)
        {
            allocator_type alloc(allocator);
            auto cb = std::allocator_traits< allocator_type >::template allocate(alloc, 1);
            std::allocator_traits< allocator_type >::template construct(
                alloc, cb, std::forward< AllocatorT >(allocator), std::forward< DeleterT >(deleter), std::forward< Args >(args)...);
            return cb;
        }

        void deallocate() override
        {
            allocator_type alloc(get_allocator());
            std::allocator_traits< allocator_type >::template destroy(alloc, this);
            std::allocator_traits< allocator_type >::template deallocate(alloc, this, 1);
        }
    };
}
