cmake_minimum_required(VERSION 3.17)

project(rpp LANGUAGES CXX)

set(SOURCES_RPP
    "alloc0.h"
    "alloc1.h"
    "array.h"
    "async.h"
    "asyncio.h"
    "base.h"
    "box.h"
    "files.h"
    "format.h"
    "function.h"
    "hash.h"
    "heap.h"
    "log.h"
    "limits.h"
    "map.h"
    "math.h"
    "net.h"
    "opt.h"
    "pair.h"
    "pool.h"
    "profile.h"
    "queue.h"
    "range_allocator.h"
    "rc.h"
    "ref0.h"
    "ref1.h"
    "reflect.h"
    "rng.h"
    "simd.h"
    "stack.h"
    "storage.h"
    "string0.h"
    "string1.h"
    "thread.h"
    "thread0.h"
    "tuple.h"
    "utility.h"
    "variant.h"
    "vec.h"
    "vmath.h"
    "std/coroutine.h"
    "std/initializer_list.h"
    "impl/alloc.cpp"
    "impl/base.cpp"
    "impl/log.cpp"
    "impl/math.cpp"
    "impl/profile.cpp"
    "impl/simd.cpp"
    "impl/vmath.cpp"
)

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

if(WIN32)
    set(SOURCES_RPP ${SOURCES_RPP}
        "w32/w32_util.h"
        "w32/w32_util.cpp"
        "w32/async_w32.cpp"
        "w32/asyncio_w32.cpp"
        "w32/thread_w32.cpp"
        "w32/files_w32.cpp"
        "w32/net_w32.cpp"
    )
elseif(LINUX)
    set(SOURCES_RPP ${SOURCES_RPP}
        "pos/async_pos.cpp"
        "pos/asyncio_pos.cpp"
        "pos/thread_pos.cpp"
        "pos/files_pos.cpp"
        "pos/net_pos.cpp"
    )
else()
    message(FATAL_ERROR "Unsupported platform: only Windows and Linux are supported.")
endif()

add_library(rpp STATIC ${SOURCES_RPP})
set_target_properties(rpp PROPERTIES CXX_STANDARD 20 CXX_EXTENSIONS OFF LINKER_LANGUAGE CXX)

target_compile_definitions(rpp
    PRIVATE
    $<$<CONFIG:Debug>:RPP_DEBUG_BUILD>
    $<$<CONFIG:RelWithDebInfo>:RPP_RELWITHDEBINFO_BUILD>
    $<$<CONFIG:Release>:RPP_RELEASE_BUILD _NO_DEBUG_HEAP=1>
    $<$<CONFIG:MinSizeRel>:RPP_RELEASE_BUILD _NO_DEBUG_HEAP=1>
)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CLANG TRUE)
endif()

if(WIN32)
    target_link_libraries(rpp PRIVATE ws2_32 Synchronization)
endif()

if(MSVC)
    if(MSVC_VERSION LESS 1937)
        message(FATAL_ERROR "Unsupported MSVC version: only 19.37+ is supported.")
    endif()

    string(REPLACE "/GR" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})

    target_compile_definitions(rpp PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX _HAS_EXCEPTIONS=0)
    target_compile_options(rpp PRIVATE /arch:AVX2 /MP /W4 /GR- /GS- /EHa- /wd4201)
elseif(CLANG)
    if(CLANG_VERSION_MAJOR LESS 17)
        message(FATAL_ERROR "Unsupported Clang version: only 17+ is supported.")
    endif()

    target_compile_options(rpp PRIVATE -mavx2 -Wall -Wextra -fno-exceptions -fno-rtti)
else()
    message(FATAL_ERROR "Unsupported compiler: only MSVC and Clang are supported.")
endif()
