//
// This file is part of smart_ptr project <https://github.com/romanpauk/smart_ptr>
//
// See LICENSE for license and copyright information
// SPDX-License-Identifier: AGPL-3.0-or-later
//

#include <thread>

#if defined(_WIN32)
#include <intrin.h>
#endif

namespace smart_ptr
{
    struct std_thread_traits
    {
        using thread_id = std::thread::id;
        static thread_id get_current_thread_id()
        {
            static thread_local thread_id id = std::this_thread::get_id();
            return id;
        }
    };

    struct std_thread_traits_uint32_t
    {
        using thread_id = uint32_t;
        static thread_id get_current_thread_id()
        {
            static thread_local thread_id id = []()
            {
                return *reinterpret_cast<thread_id*>(&std::this_thread::get_id());
            }();
            return id;
        }
    };

#if defined(_WIN32)
    struct win32_thread_traits
    {
        using thread_id = uint32_t;
        static thread_id get_current_thread_id()
        {
            // https://en.wikipedia.org/wiki/Win32_Thread_Information_Block
        #ifdef _M_IX86
            return (thread_id)__readfsdword(0x24);
        #elif _M_AMD64
            return (thread_id)__readgsqword(0x48);
        #else
        #error unsupported architecture
        #endif
        }
    };
#endif

#if defined(_WIN32)
    using default_thread_traits = win32_thread_traits;
#else
    using default_thread_traits = std_thread_traits_uint32_t;
#endif
}
