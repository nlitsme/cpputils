file(GLOB FMTLIB_DIRS ../../*/fmt /usr/include /usr/local/include /usr/local/opt/doctest/include)
find_path(FMTLIB_PATH NAMES include/fmt/printf.h PATHS ${FMTLIB_DIRS})
if (NOT FMTLIB_PATH STREQUAL "FMTLIB_PATH-NOTFOUND")
	add_library(fmtlib INTERFACE)
	target_include_directories(fmtlib INTERFACE ${FMTLIB_PATH}/include)
    target_compile_definitions(fmtlib INTERFACE FMT_HEADER_ONLY)

    add_library(fmt::fmt ALIAS fmtlib)
else()
    include(FetchContent)
    FetchContent_Declare(fmt
          GIT_REPOSITORY https://github.com/fmtlib/fmt.git
          GIT_TAG master
          )
    FetchContent_MakeAvailable(fmt)
endif()

