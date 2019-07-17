#ifdef WITH_CATCH

#define CATCH_CONFIG_ENABLE_TUPLE_STRINGMAKER
#include "contrib/catch.hpp"
#define SKIPTEST  , "[!hide]"

#elif defined(WITH_DOCTEST)

#include "contrib/doctest.h"
#define SECTION(...) SUBCASE(__VA_ARGS__)
#define SKIPTEST  * doctest::skip(true)
#define CHECK_THAT(a, b) 
#else
#error define either WITH_CATCH or WITH_DOCTEST
#endif

#include "string-strip.h"

// ... somehow doctest needs an explicit include of iostream here.
#include <iostream>

#include <string>
#include <vector>
#include <set>

template<typename V>
auto makeset(const V& v)
{
    std::set<typename V::value_type> s;
    for (auto & x : v)
        s.insert(x);
    return s;
}
TEST_CASE("strip") {

    CHECK( rstrip(std::string("abcdef"), std::string("ef")) == std::string("abcd") );
    CHECK( rstrip(std::string("abcdef"), std::string("ab")) == std::string("abcdef") );
    CHECK( lstrip(std::string("abcdef"), std::string("ef")) == std::string("abcdef") );

    CHECK( lstrip(std::string("abcdef"), "ef") == std::string("abcdef") );
    CHECK( rstrip(std::string("abcdef"), 'f') == std::string("abcde") );

    CHECK( lstrip(std::string("abcdef"), std::string("ab")) == std::string("cdef") );

    CHECK( rstrip(std::string("abcdef"), makeset(std::string("ef"))) == std::string("abcd") );
    CHECK( lstrip(std::string("abcdef"), makeset(std::string("ab"))) == std::string("cdef") );

    std::vector<char> v = { 'a', 'b', 'c', 'd', 'e', 'f' };
    CHECK( rstrip(v.begin(), v.end(), std::string("ef")) == v.begin()+4 );

    std::string txt = "xyzabcdefghi";

    CHECK( rstrip( std::string_view(&txt[3], 6), "ef") == std::string_view(&txt[3], 4) );
    CHECK( strip( std::string_view(&txt[3], 6), "abef") == std::string_view(&txt[5], 2) );
    CHECK( strip( std::string("abcdef"), "abef") == std::string("cd") );

    std::wstring wtxt = L"xyzabcdefghi";

    CHECK( rstrip( std::wstring_view(&wtxt[3], 6), "ef") == std::wstring_view(&wtxt[3], 4) );
    CHECK( strip( std::wstring_view(&wtxt[3], 6), "abef") == std::wstring_view(&wtxt[5], 2) );
    CHECK( strip( std::wstring(L"abcdef"), "abef") == std::wstring(L"cd") );



}
