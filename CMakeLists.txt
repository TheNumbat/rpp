cmake_minimum_required(VERSION 3.17)

project(rpp LANGUAGES CXX)

add_subdirectory("rpp/")
add_subdirectory("test/")

enable_testing()

file(GLOB TEST_SOURCES test/*.cpp)

foreach(test ${TEST_SOURCES})
    get_filename_component(testname ${test} NAME_WE)
    add_test(NAME ${testname}
        COMMAND $<TARGET_FILE:${testname}>
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/test")
endforeach(test ${TEST_SOURCES})
