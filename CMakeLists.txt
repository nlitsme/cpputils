cmake_minimum_required(VERSION 3.18)
project(cpputils)
set(CMAKE_CXX_STANDARD 20)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmfind")

include(boilerplate)

add_library(cpputils INTERFACE)
target_include_directories(cpputils INTERFACE .)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    find_package(doctest REQUIRED)

    file(GLOB UnittestSrc tests/*.cpp)
    if (WIN32)
        # skippoing this test on windows.
        list(REMOVE_ITEM UnittestSrc ${CMAKE_SOURCE_DIR}/tests/test-fhandle.cpp)
    endif()

    # disable work-in-progress
    list(REMOVE_ITEM UnittestSrc ${CMAKE_SOURCE_DIR}/tests/test-makeasn1.cpp)

    add_executable(unittests ${UnittestSrc})
    target_link_libraries(unittests cpputils doctest::doctest)
    target_compile_definitions(unittests PRIVATE USE_DOCTEST)

    include(CTest)
    include(doctest)
    doctest_discover_tests(unittests)
endif()

if (OPT_BENCH)
    find_package(fmt REQUIRED)
    find_package(Boost REQUIRED COMPONENTS system)
    add_executable(fmt_string-boost     fmtbench/fmt_string-boost.cpp)
    target_link_libraries(fmt_string-boost cpputils Boost::headers)

    add_executable(fmt_string-fmt       fmtbench/fmt_string-fmt.cpp)
    target_link_libraries(fmt_string-fmt cpputils fmt::fmt)
    add_executable(fmt_string-fmt2      fmtbench/fmt_string-fmt2.cpp)
    target_link_libraries(fmt_string-fmt2 cpputils fmt::fmt)
    add_executable(fmt_string-formatter fmtbench/fmt_string-formatter.cpp)
    target_link_libraries(fmt_string-formatter cpputils)
    add_executable(fmt_string-printf    fmtbench/fmt_string-printf.cpp)
    #add_executable(fmt_string-std    fmtbench/fmt_string-std.cpp)


    add_executable(fmt_print-boost     fmtbench/fmt_print-boost.cpp)
    target_link_libraries(fmt_print-boost cpputils Boost::headers)

    add_executable(fmt_print-fmt       fmtbench/fmt_print-fmt.cpp)
    target_link_libraries(fmt_print-fmt cpputils fmt::fmt)
    add_executable(fmt_print-fmt2      fmtbench/fmt_print-fmt2.cpp)
    target_link_libraries(fmt_print-fmt2 cpputils fmt::fmt)
    add_executable(fmt_print-formatter fmtbench/fmt_print-formatter.cpp)
    target_link_libraries(fmt_print-formatter cpputils)
    add_executable(fmt_print-printf    fmtbench/fmt_print-printf.cpp)
    #add_executable(fmt_print-std    fmtbench/fmt_print-std.cpp)
endif()
