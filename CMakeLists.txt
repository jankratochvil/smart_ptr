project(smart_ptr)
include(FetchContent)
cmake_minimum_required(VERSION 3.19)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS NO)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/MP>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/bigobj>")
add_link_options("$<$<CXX_COMPILER_ID:MSVC>:LINKER:/DEBUG:FASTLINK>")
enable_testing()

FetchContent_Declare(googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG main)

FetchContent_Declare(benchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG master) 
        
set(BENCHMARK_ENABLE_TESTING off)
if(WIN32)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif()
FetchContent_MakeAvailable(googletest benchmark)

add_library(smart_ptr INTERFACE)
target_include_directories(smart_ptr INTERFACE include)

target_sources(smart_ptr INTERFACE
    include/smart_ptr/shared_ptr.h
    include/smart_ptr/detail/control_block.h
    include/smart_ptr/detail/shared_counter.h
    include/smart_ptr/detail/biased_counter.h
    include/smart_ptr/detail/thread_traits.h
    README.md
)

add_executable(smart_ptr_test
    test/shared_ptr.cpp
)

add_test(smart_ptr_test COMMAND smart_ptr_test)
target_link_libraries(smart_ptr_test smart_ptr gtest_main)
target_include_directories(smart_ptr_test PRIVATE test)

add_executable(smart_ptr_benchmark
    benchmark/shared_ptr.cpp
)

target_link_libraries(smart_ptr_benchmark smart_ptr benchmark::benchmark)
target_include_directories(smart_ptr_benchmark PRIVATE benchmark)
