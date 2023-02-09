cmake_minimum_required(VERSION 3.18)
project(cpputils)
set(CMAKE_CXX_STANDARD 20)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmfind")

include(boilerplate)

add_library(cpputils INTERFACE)
target_include_directories(cpputils INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

include(CTest)
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

if (OPT_BENCH)
    add_subdirectory(fmtbench)
endif()
