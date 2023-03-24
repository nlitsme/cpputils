cmake_minimum_required(VERSION 3.18)
project(cpputils)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake_find")

include(boilerplate)

add_library(cpputils INTERFACE)
target_include_directories(cpputils INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(cpputils INTERFACE cxx_std_20)

include(CTest)
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

if (OPT_BENCH)
    add_subdirectory(fmtbench)
endif()
