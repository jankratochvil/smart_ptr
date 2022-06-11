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
#include <algorithm>

#include <smart_ptr/detail/thread_cache.h>

#include <smart_ptr/shared_ptr.h>
#include <smart_ptr/detail/shared_counter.h>

#include <queue/queue.h>

namespace smart_ptr
{
    const size_t collector_queue_size = 1 << 12;
    using collector_message = uintptr_t;
    
    class collector_queue: public queue::bounded_queue_spsc3< collector_message, queue::static_storage2< collector_message, collector_queue_size > >
    {
    public:
        void set_released(bool released) { released_ = released; }
        bool is_released() const { return released_; }
    private:
        bool released_ = false;
    };

    using collector_queue_ptr = shared_ptr< collector_queue, shared_counter< uint64_t, true > >;

    struct collector
    {
        template < typename T > struct handle
        {
            template < typename... Args > handle(Args&&... args)
                : value(std::forward< Args >(args)...)
            {
                value = instance().acquire_queue();
            }

            ~handle()
            {
                instance().release_queue(value);
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
            static thread_local handle< collector_queue* > handle;
            return *handle.value;
        }

        collector()
        {
            thread_ = std::thread([&]
            {
                while (!dtor_)
                {
                    drain();
                }
            });
        }

        ~collector()
        {            
            dtor_ = true;
            thread_.join();

            while(drain());
        }

        collector_queue* acquire_queue()
        {
            std::lock_guard< std::mutex > lock(mutex_);

            queues_.push_back(make_shared< collector_queue, shared_counter< uint64_t, true > >());
            return queues_.back().get();
        }

        void release_queue(collector_queue* queue)
        {
            std::lock_guard< std::mutex > lock(mutex_);

            auto it = std::find_if(queues_.begin(), queues_.end(), [=](auto value){ return value.get() == queue; });
            assert(it != queues_.end());
            (*it)->set_released(true);
        }

    private:
        size_t drain()
        {
            {
                // During this lock, threads cannot join or exit the collector.
                std::lock_guard< std::mutex > lock(mutex_);

                // Copy all queues
                tmp_queues_ = queues_;

                // Remove released queues from queues_. This will drain them one last time.
                queues_.erase(std::remove_if(queues_.begin(), queues_.end(), [](auto queue) { return queue->is_released(); }), queues_.end());
            }

            for (auto& queue : tmp_queues_)
            {
                auto size = queue->pop<false>(tmp_messages_);
                for (size_t i = 0; i < size; ++i)
                {
                    auto ptr = (control_block_dtor*)(tmp_messages_[size] & ~1);
                    auto inc = tmp_messages_[size] & 1;

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
                            tmp_zeroes_.push_back(ptr);
                        }
                    }
                }
            }

            size_t deallocated = 0;
            for (auto ptr : tmp_zeroes_)
            {
                auto it = control_blocks_.find(ptr);
                assert(it != control_blocks_.end());
                if (it->second == 0)
                {
                    it->first->deallocate();
                    it = control_blocks_.erase(it);
                    ++deallocated;
                }
                else
                {
                    ++it;
                }
            }

            tmp_zeroes_.clear();
            return deallocated;
        }

        // Accessed from multiple threads
        alignas(64) std::mutex mutex_;
        std::thread thread_;
        std::atomic< bool > dtor_ = false;
        std::vector< collector_queue_ptr > queues_;

        // Accessed from single thread
        alignas(64) std::unordered_map< control_block_dtor*, uint64_t > control_blocks_;

        // Temporary data
        std::vector< control_block_dtor* > tmp_zeroes_;
        std::vector< collector_queue_ptr > tmp_queues_;
        std::array< collector_message, collector_queue_size > tmp_messages_;
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
