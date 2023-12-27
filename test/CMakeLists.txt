cmake_minimum_required(VERSION 3.17)

project(rpp_test LANGUAGES CXX)

add_executable(rpp_test "test.cpp")
set_target_properties(rpp_test PROPERTIES CXX_STANDARD 20 CXX_EXTENSIONS OFF LINKER_LANGUAGE CXX)

target_link_libraries(rpp_test PRIVATE rpp)
target_include_directories(rpp_test PRIVATE "..")

if(MSVC)
    target_compile_options(rpp_test PRIVATE /arch:AVX2)
else()
    target_compile_options(rpp_test PRIVATE -mavx2)
endif()
