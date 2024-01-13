cmake_minimum_required(VERSION 3.17)

project(rpp LANGUAGES CXX)

option(RPP_TEST "Build Tests" OFF)

add_subdirectory("rpp/")

if(RPP_TEST)
    enable_testing()
    add_subdirectory("test/")

    file(GLOB TEST_SOURCES test/*.cpp)

    foreach(test ${TEST_SOURCES})
        get_filename_component(testname ${test} NAME_WE)
        add_test(NAME ${testname}
            COMMAND $<TARGET_FILE:${testname}>
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/test")
    endforeach(test ${TEST_SOURCES})
endif()
