find_package(fmt REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)
add_executable(fmt_string-boost     fmt_string-boost.cpp)
target_link_libraries(fmt_string-boost cpputils Boost::headers)

add_executable(fmt_string-fmt       fmt_string-fmt.cpp)
target_link_libraries(fmt_string-fmt cpputils fmt::fmt)
add_executable(fmt_string-fmt2      fmt_string-fmt2.cpp)
target_link_libraries(fmt_string-fmt2 cpputils fmt::fmt)
add_executable(fmt_string-formatter fmt_string-formatter.cpp)
target_link_libraries(fmt_string-formatter cpputils)
add_executable(fmt_string-printf    fmt_string-printf.cpp)
#add_executable(fmt_string-std    fmt_string-std.cpp)


add_executable(fmt_print-boost     fmt_print-boost.cpp)
target_link_libraries(fmt_print-boost cpputils Boost::headers)

add_executable(fmt_print-fmt       fmt_print-fmt.cpp)
target_link_libraries(fmt_print-fmt cpputils fmt::fmt)
add_executable(fmt_print-fmt2      fmt_print-fmt2.cpp)
target_link_libraries(fmt_print-fmt2 cpputils fmt::fmt)
add_executable(fmt_print-formatter fmt_print-formatter.cpp)
target_link_libraries(fmt_print-formatter cpputils)
add_executable(fmt_print-printf    fmt_print-printf.cpp)
#add_executable(fmt_print-std    fmt_print-std.cpp)

