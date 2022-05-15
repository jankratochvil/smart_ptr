project(biased_shared_ptr)

cmake_minimum_required(VERSION 3.19)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/MP>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/bigobj>")
add_link_options("$<$<CXX_COMPILER_ID:MSVC>:LINKER:/DEBUG:FASTLINK>")
enable_testing()

add_library(biased_shared_ptr INTERFACE)
target_include_directories(biased_shared_ptr INTERFACE include)

target_sources(biased_shared_ptr INTERFACE
    include/biased_shared_ptr.h
    readme.md
)

macro(group_sources path)
    file(GLOB children RELATIVE ${PROJECT_SOURCE_DIR}/${path} ${PROJECT_SOURCE_DIR}/${path}/*)
    foreach(child ${children})
        if(IS_DIRECTORY ${PROJECT_SOURCE_DIR}/${path}/${child})
            group_sources(${path}/${child})
        else()
            string(REPLACE "/" "\\" groupname ${path})
            source_group(${groupname} FILES ${PROJECT_SOURCE_DIR}/${path}/${child})
        endif()
    endforeach()
endmacro()

find_package(Boost)
if(Boost_FOUND)
    add_executable(biased_shared_ptr_test
        test/pch.h
        test/biased_shared_ptr.cpp
        test/testmodule.cpp
    )

    add_test(biased_shared_ptr_test COMMAND biased_shared_ptr_test)
    target_link_libraries(biased_shared_ptr_test biased_shared_ptr Boost::boost)
    target_include_directories(biased_shared_ptr_test PRIVATE test)
    target_precompile_headers(biased_shared_ptr_test PRIVATE test/pch.h)

    if(WIN32)
        group_sources(include)
        group_sources(test)
    endif()
endif()
