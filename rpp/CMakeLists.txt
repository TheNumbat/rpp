cmake_minimum_required(VERSION 3.17)

project(rpp LANGUAGES CXX)

set(SOURCES_RPP
    "alloc.cpp"
    "alloc0.h"
    "alloc1.h"
    "array.h"
    "async.h"
    "asyncio.h"
    "base.cpp"
    "base.h"
    "box.h"
    "files.h"
    "format.h"
    "function.h"
    "hash.h"
    "heap.h"
    "log.cpp"
    "log.h"
    "map.h"
    "math.cpp"
    "math.h"
    "net.h"
    "opt.h"
    "pair.h"
    "pool.h"
    "profile.cpp"
    "profile.h"
    "queue.h"
    "range_allocator.h"
    "rc.h"
    "ref0.h"
    "ref1.h"
    "reflect.h"
    "rng.h"
    "simd.cpp"
    "simd.h"
    "stack.h"
    "storage.h"
    "string0.h"
    "string1.h"
    "thread.h"
    "thread0.h"
    "tuple.h"
    "variant.h"
    "vec.h"
    "vmath.cpp"
    "vmath.h"
)

if(WIN32)
    set(SOURCES_RPP ${SOURCES_RPP}
        "w32_util.h"
        "w32_util.cpp"
        "async_w32.cpp"
        "asyncio_w32.cpp"
        "thread_w32.cpp"
        "files_w32.cpp"
        "net_w32.cpp"
    )
endif()

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

if(APPLE OR LINUX)
    set(SOURCES_RPP ${SOURCES_RPP}
        "async_pos.cpp"
        "asyncio_pos.cpp"
        "thread_pos.cpp"
        "files_pos.cpp"
        "net_pos.cpp"
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

if(WIN32)
    target_link_libraries(rpp_test PRIVATE ws2_32)
endif()
