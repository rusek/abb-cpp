cmake_minimum_required(VERSION 2.8.12.2)
project(abb)

enable_testing()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -pthread")
set(CMAKE_CXX_FLAGS_COVERAGE "-g -O0 --coverage")

include_directories("include")

file(GLOB DEMO_SOURCES "demo/main.cpp" "src/*.cpp")
add_executable(demo ${DEMO_SOURCES})

file(GLOB TEST_SOURCES "tests/*.cpp")
foreach(TEST_SOURCE ${TEST_SOURCES})
    get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)
    file(GLOB SINGLE_TEST_SOURCES ${TEST_SOURCE} "src/*.cpp" "tests/helpers/*.cpp")
    add_executable("test_${TEST_NAME}" ${SINGLE_TEST_SOURCES})
    add_test("test_${TEST_NAME}" "test_${TEST_NAME}")
endforeach()
