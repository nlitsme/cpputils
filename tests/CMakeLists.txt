find_package(doctest REQUIRED)

file(GLOB UnittestSrc *.cpp)
if (WIN32)
    # skippoing these tests on windows.
    list(REMOVE_ITEM UnittestSrc test-fhandle.cpp)
    list(REMOVE_ITEM UnittestSrc test-mmem.cpp)
endif()

# disable work-in-progress
list(REMOVE_ITEM UnittestSrc test-makeasn1.cpp)

add_executable(cpputils_unittests ${UnittestSrc})
target_link_libraries(cpputils_unittests cpputils doctest::doctest)
target_compile_definitions(cpputils_unittests PRIVATE USE_DOCTEST)

include(CTest)
include(doctest)
doctest_discover_tests(cpputils_unittests)
