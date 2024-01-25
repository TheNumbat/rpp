cmake_minimum_required(VERSION 3.17)

project(rpp LANGUAGES CXX)

option(RPP_TEST "Build tests" OFF)
option(RPP_QEMU "Run tests with qemu-aarch64" OFF)

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

        if(RPP_QEMU)
            set(command qemu-aarch64 -L /usr/aarch64-linux-gnu/ $<TARGET_FILE:${testname}>)
        else()
            set(command $<TARGET_FILE:${testname}>)
        endif()

        add_test(NAME ${testname}
            COMMAND ${command}
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/test")
    endforeach(test ${TEST_SOURCES})
endif()
