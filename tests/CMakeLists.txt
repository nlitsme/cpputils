find_package(doctest REQUIRED)

file(GLOB UnittestSrc *.cpp)
if (WIN32)
    # skippoing this test on windows.
    list(REMOVE_ITEM UnittestSrc test-fhandle.cpp)
endif()

# disable work-in-progress
list(REMOVE_ITEM UnittestSrc test-makeasn1.cpp)

add_executable(unittests ${UnittestSrc})
target_link_libraries(unittests cpputils doctest::doctest)
target_compile_definitions(unittests PRIVATE USE_DOCTEST)

include(CTest)
include(doctest)
doctest_discover_tests(unittests)
