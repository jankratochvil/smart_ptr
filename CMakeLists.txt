project(smart_ptr)
include(FetchContent)
cmake_minimum_required(VERSION 3.19)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS NO)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

enable_testing()

option(SMARTPTR_ENABLE_TESTING "enable testing through googletest" ON)
option(SMARTPTR_ENABLE_BENCHMARK "enable benchmarking through googlebenchmark" ON)

if(SMARTPTR_ENABLE_TESTING)
    FetchContent_Declare(googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG main)

    if(WIN32)
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    endif()

    FetchContent_MakeAvailable(googletest)
endif()

if(SMARTPTR_ENABLE_BENCHMARK)
    FetchContent_Declare(benchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG master) 
       
    set(BENCHMARK_ENABLE_TESTING off)
    FetchContent_MakeAvailable(benchmark)
endif()

FetchContent_Declare(queue
    GIT_REPOSITORY git@github.com:romanpauk/queue.git
    GIT_TAG develop)

set(QUEUE_ENABLE_TESTING OFF)
set(QUEUE_ENABLE_BENCHMARK OFF)

FetchContent_MakeAvailable(queue)

add_library(smart_ptr INTERFACE)
target_include_directories(smart_ptr INTERFACE include)

target_sources(smart_ptr INTERFACE
    include/smart_ptr/shared_ptr.h
    include/smart_ptr/detail/control_block.h
    include/smart_ptr/detail/shared_counter.h
    include/smart_ptr/detail/biased_counter.h
    include/smart_ptr/detail/thread_cache.h
    include/smart_ptr/detail/thread_counter.h
    include/smart_ptr/detail/thread_traits.h
    README.md
)

if(SMARTPTR_ENBALE_TESTING)
    add_executable(smart_ptr_test
        test/shared_ptr.cpp
    )

    add_test(smart_ptr_test COMMAND smart_ptr_test)
    target_link_libraries(smart_ptr_test smart_ptr gtest_main queue)
    target_include_directories(smart_ptr_test PRIVATE test)
endif()

if(SMARTPTR_ENABLE_BENCHMARK)
    add_executable(smart_ptr_benchmark
        benchmark/shared_ptr.cpp
    )

    target_link_libraries(smart_ptr_benchmark smart_ptr benchmark::benchmark queue)
    target_include_directories(smart_ptr_benchmark PRIVATE benchmark)
endif()
