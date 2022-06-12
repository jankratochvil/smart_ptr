//
// This file is part of smart_ptr project <https://github.com/romanpauk/smart_ptr>
//
// See LICENSE for license and copyright information
// SPDX-License-Identifier: AGPL-3.0-or-later
//

#pragma once

#include <smart_ptr/detail/thread_cache.h>
#include <smart_ptr/shared_ptr.h>
#include <smart_ptr/detail/shared_counter.h>

#include <queue/queue.h>

#include <thread>
#include <mutex>
#include <vector>
#include <unordered_map>
#include <cassert>
#include <algorithm>
#include <new>

namespace smart_ptr
{
    const size_t collector_queue_size = 1 << 12;
    using collector_message = uintptr_t;
    
    class collector_queue: public queue::bounded_queue_spsc2< collector_message, queue::static_storage< collector_message, collector_queue_size > >
    {
    public:
        void set_released(bool released) { released_ = released; }
        bool is_released() const { return released_; }
    private:
        bool released_ = false;
    };

    // Collector queue is not used without lock from different threads, so it uses non-atomic shared_ptr counter
    using collector_queue_ptr = shared_ptr< collector_queue, shared_counter< uint64_t, false > >;

    class collector
    {
    public:
        collector()
        {
            thread_ = std::thread([&]
            {
                drain_state state;
                while (!dtor_)
                {
                    drain(state);
                }
            });
        }

        ~collector()
        {            
            dtor_ = true;
            thread_.join();

            drain_state state;
            while(drain(state));
        }

        static collector& instance()
        {
            static collector value;
            return value;
        }

        void push(collector_message msg)
        {
            queue().push(msg);
        }

    private:
        static collector_queue& queue()
        {
            static thread_local handle< collector_queue* > handle;
            return *handle.value;
        }

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

        collector_queue* acquire_queue()
        {
            std::lock_guard< std::mutex > lock(mutex_);

            queues_.push_back(make_shared< collector_queue, shared_counter< uint64_t, false > >());
            return queues_.back().get();
        }

        void release_queue(collector_queue* queue)
        {
            std::lock_guard< std::mutex > lock(mutex_);

            auto it = std::find_if(queues_.begin(), queues_.end(), [=](auto value) { return value.get() == queue; });
            assert(it != queues_.end());
            (*it)->set_released(true);
        }

        struct drain_state
        {
            std::vector< collector_queue_ptr > queues;
            std::vector< control_block_dtor* > zeroes;
            std::array< collector_message, collector_queue_size > messages;
        };        

        size_t drain(drain_state& state)
        {
            {
                // During this lock, threads cannot join or exit the collector.
                std::lock_guard< std::mutex > lock(mutex_);

                // Copy all queues
                state.queues = queues_;

                // Remove released queues from queues_. This will drain them one last time.
                queues_.erase(std::remove_if(queues_.begin(), queues_.end(), [](auto queue) { return queue->is_released(); }), queues_.end());
            }

            size_t size = 0;
            for (auto& queue : state.queues)
            {
                while (size = queue->pop<false>(state.messages))
                {
                    for (size_t i = 0; i < size; ++i)
                    {
                        auto ptr = (control_block_dtor*)(state.messages[size] & ~1);
                        auto inc = state.messages[size] & 1;

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
                                state.zeroes.push_back(ptr);
                            }
                        }
                    }
                }
            }

            size_t deallocated = 0;
            for (auto ptr : state.zeroes)
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

            state.zeroes.clear();
            state.queues.clear();

            return deallocated;
        }

        // Accessed from multiple threads
        alignas(64) std::mutex mutex_;
        std::thread thread_;
        std::atomic< bool > dtor_ = false;
        std::vector< collector_queue_ptr > queues_;

        // Accessed from single thread
        alignas(64) std::unordered_map< control_block_dtor*, uint64_t > control_blocks_;
    };

    template < typename T, typename ThreadCache > struct thread_counter
    {
        thread_counter(control_block_dtor* cb)
        {
            collector::instance().push((uintptr_t)cb & 1);
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
            
            collector::instance().push((uintptr_t)cb & 1);
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
            
            collector::instance().push((uintptr_t)cb);

            // Always return false as the destruction is done from the collector thread
            return false;
        }

    private:
        ThreadCache cache_;
    };
}
