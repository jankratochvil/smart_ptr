//
// This file is part of smart_ptr project <https://github.com/romanpauk/smart_ptr>
//
// See LICENSE for license and copyright information
// SPDX-License-Identifier: AGPL-3.0-or-later
//

#pragma once

#include <thread>
#include <mutex>
#include <vector>
#include <unordered_map>
#include <cassert>

#include <smart_ptr/detail/thread_cache.h>
#include <queue/queue.h>

namespace smart_ptr
{
    using collector_message = void*; //std::pair< void*, bool >;
    using collector_queue_storage = queue::static_storage2< collector_message, 1 << 16 >;
    using collector_queue = queue::bounded_queue_spsc3< collector_message, collector_queue_storage >;

    template < typename T > struct collector
    {
        collector()
        {
            thread_ = std::thread([&]
            {
                std::array< collector_message, 1 << 15 > messages;

                while (!dtor_)
                {
                    {
                        /*
                        std::vector< collector_queue* > queues;

                        {
                            
                            queues = std::move(queues_);
                        }
                        */

                        std::lock_guard< std::mutex > lock(mutex_);
                        for (auto& queue : queues_)
                        {                            
                            auto size = queue->pop(messages);
                            for (size_t i = 0; i < size; ++i)
                            {
                                //auto ptr = messages[size] & ~1;
                                //auto inc = messages[size] & 1;
                            }

                            // TODO: remember zeroes...
                            //control_blocks_[element.first] += element.second ? 1 : -1;
                        }
                    }
/*
                    for (auto it = control_blocks_.begin(); it != control_blocks_.end(); ++it)
                    {
                        if (it->second == 0)
                        {                            
                            //delete it->first;
                            it = control_blocks_.erase(it);
                        }
                        else
                        {
                            ++it;
                        }
                    }
*/
                }
            });
        }

        ~collector()
        {
            dtor_ = false;
            thread_.join();
        }

        void register_queue(collector_queue& queue)
        {
            std::lock_guard< std::mutex > lock(mutex_);
            queues_.push_back(&queue);
        }

        void remove_queue(collector_queue& queue)
        {
            std::lock_guard< std::mutex > lock(mutex_);
            auto it = std::find(queues_.begin(), queues_.end(), &queue);
            assert(it != queues_.end());
            if (it != queues_.end())
            {
                queues_.erase(it);
            }
        }

    private:
        std::mutex mutex_;
        std::thread thread_;
        std::atomic< bool > dtor_;

        // TODO: if this is shared_ptr, we can unlock while processing.
        // But as it is thread local, no threads can go during that time.
        std::vector< collector_queue* > queues_;

        std::unordered_map< void*, size_t > control_blocks_;
    };

    template < typename T > struct collector_queue_handle
    {
        template < typename... Args > collector_queue_handle(Args&&... args)
            : value(std::forward< Args >(args)...)
        {
            get_collector().register_queue(value);
        }

        ~collector_queue_handle()
        {
            get_collector().remove_queue(value);
        }

        static collector< T >& get_collector()
        {
            static collector< T > value;
            return value;
        }

        T value;
    };

    template< typename T > auto& get_collector_queue()
    {
        static thread_local collector_queue_storage storage;
        static thread_local collector_queue_handle< collector_queue > handle(storage);
        return handle.value;
    }

    template < typename T > struct thread_counter
    {
        thread_counter()
        {
            get_collector_queue< control_block_base< uint64_t, thread_counter > >().push(this);
        }

        ~thread_counter()
        {}

        void increment()
        {
            auto index = cache_.get((uintptr_t)this);
            if (index != cache_.end())
            {
                ++cache_[index];
                return;
            }

            get_collector_queue< control_block_base< uint64_t, thread_counter > >().push(this);
        }

        bool decrement()
        {
            auto index = cache_.get((uintptr_t)this);
            if (index != cache_.end())
            {
                if(cache_[index]-- > 0)
                    return false;

                cache_.erase(index);
            }

            get_collector_queue< control_block_base< uint64_t, thread_counter > >().push(this);
            return false;
        }

    private:
        thread_cache< uintptr_t, T, 8 > cache_;
    };
}
