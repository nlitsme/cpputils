cmake_minimum_required(VERSION 3.10)
project(cpputils)
set(CMAKE_CXX_STANDARD 17)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(LINUX TRUE)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(DARWIN TRUE)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
    set(FREEBSD TRUE)
endif()

# checking for if we are called in the correct way:
#  with a -B argument.  and without a cache file in the source directory.
if (CMAKE_CACHEFILE_DIR STREQUAL "${CMAKE_SOURCE_DIR}")
    message(FATAL_ERROR "\nUnexpected CMakeCache.txt file in the source directory. Please remove it.")
    return()
endif()

if (EXISTS ${CMAKE_BINARY_DIR}/CMakeLists.txt)
    message(FATAL_ERROR "\nRun cmake with an explicit -B buildpath")
    return()
endif()

if(MSVC)
    # /MP = multithreaded build
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
    # /utf-8 = utf8 source and execution
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
endif()

if (WIN32)
    add_definitions(-DNOMINMAX -DNOGDI)
endif()

add_library(cpputils INTERFACE)
target_include_directories(cpputils INTERFACE .)

file(GLOB UnittestSrc tests/*.cpp)
if (WIN32)
    # skippoing this test on windows.
    list(REMOVE_ITEM UnittestSrc ${CMAKE_SOURCE_DIR}/tests/test-fhandle.cpp)
endif()

# disable work-in-progress
list(REMOVE_ITEM UnittestSrc ${CMAKE_SOURCE_DIR}/tests/test-makeasn1.cpp)

add_executable(unittests ${UnittestSrc})
target_link_libraries(unittests cpputils)
target_compile_definitions(unittests PRIVATE WITH_DOCTEST)
