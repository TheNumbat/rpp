cmake_minimum_required(VERSION 3.17)

project(rpp LANGUAGES CXX)

set(SOURCES_RPP
    "3d_math.h"
    "alloc.cpp"
    "alloc0.h"
    "alloc1.h"
    "array.h"
    "base.h"
    "box.h"
    "format.h"
    "log.cpp"
    "log.h"
    "math.h"
    "opt.h"
    "pair.h"
    "ref0.h"
    "ref1.h"
    "reflect.h"
    "stack.h"
    "storage.h"
    "string0.h"
    "string1.h"
    "thread0.h"
    "vec.h"
    "stack.h"
    "queue.h"
    "heap.h"
    "map.h"
    "profile.cpp"
    "profile.h"
    "rng.h"
    "range_allocator.h"
    "range_allocator.inl"
    "files.h"
    "net.h"
    "rc.h"
    "pool.h"
    "async.h"
    "tuple.h"
    "variant.h"
    "function.h"
)

if(WIN32)
    set(SOURCES_RPP ${SOURCES_RPP} "thread_w32.cpp" "files_w32.cpp" "net_w32.cpp")
endif()

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

if(APPLE OR LINUX)
    set(SOURCES_RPP ${SOURCES_RPP} "thread_pos.cpp" "files_pos.cpp" "net_pos.cpp")
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
    target_compile_definitions(rpp PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
    target_compile_options(rpp PRIVATE /MP /W4 /WX /GS- /wd4201 /wd4840 /wd4100 /arch:AVX2)
else()
    target_compile_options(rpp PRIVATE -mavx2 -Wall -Wextra -Werror -Wno-reorder -Wno-missing-braces -Wno-unused-parameter)
endif()

if(WIN32)
    target_link_libraries(rpp_test PRIVATE ws2_32)
endif()
