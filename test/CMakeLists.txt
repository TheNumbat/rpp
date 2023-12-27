cmake_minimum_required(VERSION 3.17)

project(rpp_tests LANGUAGES CXX)

file(GLOB TEST_SOURCES *.cpp)

foreach(test ${TEST_SOURCES})
    get_filename_component(testname ${test} NAME_WE)
    add_executable(${testname} ${test})

    set_target_properties(${testname} PROPERTIES CXX_STANDARD 20 CXX_EXTENSIONS OFF LINKER_LANGUAGE CXX)
    target_link_libraries(${testname} PRIVATE rpp)
    target_include_directories(${testname} PRIVATE "../rpp")

    if(MSVC)
        target_compile_options(${testname} PRIVATE /arch:AVX2)
    else()
        target_compile_options(${testname} PRIVATE -mavx2)
    endif()
endforeach(test ${TEST_SOURCES})
