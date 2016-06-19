cmake_minimum_required(VERSION 2.8.12.2)
project(abb)

enable_testing()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -pthread")
set(CMAKE_CXX_FLAGS_COVERAGE "-g -O0 --coverage")
set(CMAKE_CXX_FLAGS_MEMCHECK "-g -O0")

include_directories("include")

file(GLOB ABB_SOURCES "src/*.cpp")
add_library(lib_abb OBJECT ${ABB_SOURCES})

file(GLOB DEMO_HELPERS_SOURCES "demos/helpers/*.cpp")
add_library(lib_demo_helpers OBJECT ${DEMO_HELPERS_SOURCES})

file(GLOB DEMO_SOURCES "demos/*.cpp")
foreach(DEMO_SOURCE ${DEMO_SOURCES})
    get_filename_component(DEMO_NAME ${DEMO_SOURCE} NAME_WE)
    add_executable("demo_${DEMO_NAME}" ${DEMO_SOURCE} $<TARGET_OBJECTS:lib_demo_helpers> $<TARGET_OBJECTS:lib_abb>)
endforeach()

file(GLOB TEST_HELPERS_SOURCES "tests/helpers/*.cpp")
add_library(lib_test_helpers OBJECT ${TEST_HELPERS_SOURCES})

if(CMAKE_BUILD_TYPE STREQUAL "Memcheck")
    set(TEST_PREFIX "valgrind" "--leak-check=full" "--error-exitcode=1")
else()
    set(TEST_PREFIX )
endif(CMAKE_BUILD_TYPE STREQUAL "Memcheck")

file(GLOB TEST_SOURCES "tests/*.cpp")
foreach(TEST_SOURCE ${TEST_SOURCES})
    get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)
    add_executable("test_${TEST_NAME}" ${TEST_SOURCE} $<TARGET_OBJECTS:lib_test_helpers> $<TARGET_OBJECTS:lib_abb>)
    add_test("test_${TEST_NAME}" ${TEST_PREFIX} ./test_${TEST_NAME})
endforeach()
