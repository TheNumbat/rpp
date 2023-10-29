cmake_minimum_required(VERSION 3.17)

project(rpp_test_test LANGUAGES CXX)

set(SOURCES_RPP_TEST
    "test.cpp")

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

add_executable(rpp_test ${SOURCES_RPP_TEST})
set_target_properties(rpp_test PROPERTIES CXX_STANDARD 20 CXX_EXTENSIONS OFF LINKER_LANGUAGE CXX)

add_subdirectory("rpp/")
target_link_libraries(rpp_test PRIVATE rpp)

target_compile_definitions(rpp_test
    PRIVATE
    $<$<CONFIG:Debug>:DEBUG_BUILD>
    $<$<CONFIG:RelWithDebInfo>:RELWITHDEBINFO_BUILD>
    $<$<CONFIG:Release>:RELEASE_BUILD>
    $<$<CONFIG:MinSizeRel>:RELEASE_BUILD>
)

if(MSVC)
    target_compile_definitions(rpp_test PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
    target_compile_options(rpp_test PRIVATE /MP /W4 /WX /wd4201 /wd4840 /wd4100 /arch:AVX2)
else()
    target_compile_options(rpp_test PRIVATE -mavx2 -Wall -Wextra -Werror -Wno-reorder -Wno-missing-braces -Wno-unused-parameter)

    set_target_properties(rpp_test PROPERTIES
        LINK_FLAGS -fsanitize=thread
        COMPILE_FLAGS -fsanitize=thread -fno-omit-frame-pointer -fno-optimize-sibling-calls)
endif()

if(WIN32)
    target_link_libraries(rpp_test PRIVATE ws2_32 Synchronization)
endif()
