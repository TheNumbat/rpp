cmake_minimum_required(VERSION 3.17)

project(rpp LANGUAGES CXX)

add_subdirectory("rpp/")
add_subdirectory("test/")

enable_testing()
add_test(NAME rpp_test COMMAND $<TARGET_FILE:rpp_test>)
