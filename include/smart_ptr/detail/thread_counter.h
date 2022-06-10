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
    const size_t collector_queue_size = 1 << 12;
    using collector_message = uintptr_t;
    using collector_queue_storage = queue::static_storage2< collector_message, collector_queue_size >;
    using collector_queue = queue::bounded_queue_spsc3< collector_message, collector_queue_storage >;

    struct collector
    {
        template < typename T > struct handle
        {
            template < typename... Args > handle(Args&&... args)
                : value(std::forward< Args >(args)...)
            {
                instance().register_queue(value);
            }

            ~handle()
            {
                instance().remove_queue(value);
            }

            T value;
        };

        static collector& instance()
        {
            static collector value;
            return value;
        }

        static auto& queue()
        {            
            static thread_local handle< collector_queue > handle;
            return handle.value;
        }

        collector()
        {
            thread_ = std::thread([&]
            {
                // This is all very crude...

                std::array< collector_message, collector_queue_size > messages;
                std::vector< control_block_dtor* > zeroes;

                while (!dtor_)
                {
                    {
                        std::lock_guard< std::mutex > lock(mutex_);

                        for (auto& queue : queues_)
                        {
                            auto size = queue->pop<false>(messages);
                            for (size_t i = 0; i < size; ++i)
                            {
                                auto ptr = (control_block_dtor*)(messages[size] & ~1);
                                auto inc = messages[size] & 1;
                                
                                auto& cnt = control_blocks_[ptr];
                                if (inc)
                                {
                                    cnt += 1;
                                }
                                else
                                {
                                    assert(cnt >= 0);
                                    cnt -= 1;
                                    if (cnt == 0)
                                    {
                                        zeroes.push_back(ptr);
                                    }
                                }
                            }
                        }
                    }
                    
                    for (auto ptr: zeroes)
                    {
                        auto it = control_blocks_.find(ptr);
                        assert(it != control_blocks_.end());
                        if (it->second == 0)
                        {
                            it->first->deallocate();
                            it = control_blocks_.erase(it);
                        }
                        else
                        {
                            ++it;
                        }
                    }

                    zeroes.clear();
                }
            });
        }

        ~collector()
        {
            // TODO: delete the remaining control blocks
            dtor_ = true;
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
        std::atomic< bool > dtor_ = false;

        // TODO: if this is shared_ptr, we can unlock while processing.
        // But as it is thread local, no threads can go during that time.
        // Even better would be for collector to own the queues and threads would
        // just borrow them.
        std::vector< collector_queue* > queues_;

        std::unordered_map< control_block_dtor*, uint64_t > control_blocks_;
    };

    template < typename T, typename ThreadCache > struct thread_counter
    {
        thread_counter(control_block_dtor* cb)
        {
            collector::queue().push((uintptr_t)cb & 1);
        }

        ~thread_counter()
        {}

        void increment(control_block_dtor* cb)
        {
            // It would be interesting to be able to use LRU eviction here. But that is complicated because of
            // refcounts would be kept on two places. Now they are either in the cache or in the collector and cannot move once placed.
            auto index = cache_.get((uintptr_t)this);
            if (index != cache_.end())
            {
                if (cache_[index]++ == 0)
                {
                    // Very slow on this place.
                    //collector::queue().push(cb);
                }

                return;
            }
            
            collector::queue().push((uintptr_t)cb & 1);
        }

        bool decrement(control_block_dtor* cb)
        {
            auto index = cache_.get((uintptr_t)this);
            if (index != cache_.end())
            {
                if(cache_[index]-- > 0)
                    return false;

                cache_.erase(index);
            }

            collector::queue().push((uintptr_t)cb);

            // Always return false as the destruction is done from collector thread.
            return false;
        }

    private:
        ThreadCache cache_;
    };
}
