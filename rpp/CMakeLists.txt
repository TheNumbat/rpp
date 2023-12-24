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
endif()

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

if(APPLE OR LINUX)
    set(SOURCES_RPP ${SOURCES_RPP}
        "pos/async_pos.cpp"
        "pos/asyncio_pos.cpp"
        "pos/thread_pos.cpp"
        "pos/files_pos.cpp"
        "pos/net_pos.cpp"
    )
endif()

add_library(rpp STATIC ${SOURCES_RPP})
set_target_properties(rpp PROPERTIES CXX_STANDARD 20 CXX_EXTENSIONS OFF LINKER_LANGUAGE CXX)

target_compile_definitions(rpp
    PRIVATE
    $<$<CONFIG:Debug>:DEBUG_BUILD>
    $<$<CONFIG:RelWithDebInfo>:RELWITHDEBINFO_BUILD>
    $<$<CONFIG:Release>:RELEASE_BUILD>
    $<$<CONFIG:MinSizeRel>:RELEASE_BUILD>
)

if(MSVC)
    string(REPLACE "/GR" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
    target_compile_definitions(rpp PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX _HAS_EXCEPTIONS=0)
    target_compile_options(rpp PRIVATE /MP /W4 /WX /GR- /GS- /EHa- /wd4201 /wd4840 /wd4100 /arch:AVX2)
else()
    target_compile_options(rpp PRIVATE -mavx2 -Wall -Wextra -Werror -Wno-reorder -Wno-missing-braces -Wno-unused-parameter -fno-exceptions -fno-rtti)
endif()
