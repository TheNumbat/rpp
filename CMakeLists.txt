cmake_minimum_required(VERSION 3.17)

project(rpp LANGUAGES CXX)

option(RPP_TEST "Build tests" OFF)

add_subdirectory("rpp/")

get_directory_property(HAS_PARENT PARENT_DIRECTORY)

if(HAS_PARENT)
    set(RPP_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)
endif()

if(RPP_TEST)
    set(RPP_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR})

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
