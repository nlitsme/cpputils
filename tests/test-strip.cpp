#include "unittestframework.h"

#include "string-strip.h"
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
    CHECK( rstrip(std::string("abcdef"), std::string("")) == std::string("abcdef") );

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

    CHECK( rstrip(std::string("abc"), std::string("bc")) == std::string("a") );
    CHECK( rstrip(std::string("abc"), std::string("b")) == std::string("abc") );
    CHECK( rstrip(std::string("abc"), std::string("")) == std::string("abc") );
    CHECK( rstrip(std::string("ab"), std::string("b")) == std::string("a") );
    CHECK( rstrip(std::string("a"), std::string("b")) == std::string("a") );
    CHECK( rstrip(std::string("ab"), std::string("ab")) == std::string("") );
    CHECK( rstrip(std::string("a"), std::string("ab")) == std::string("") );
    CHECK( rstrip(std::string(""), std::string("ab")) == std::string("") );
    CHECK( rstrip(std::string(""), std::string("a")) == std::string("") );
    CHECK( rstrip(std::string(""), std::string("")) == std::string("") );

    CHECK( lstrip(std::string("fedcba"), std::string("fe")) == std::string("dcba") );
    CHECK( lstrip(std::string("fedcba"), std::string("ba")) == std::string("fedcba") );
    CHECK( lstrip(std::string("fedcba"), std::string("")) == std::string("fedcba") );

    CHECK( lstrip(std::string("cba"), std::string("cb")) == std::string("a") );
    CHECK( lstrip(std::string("cba"), std::string("b")) == std::string("cba") );
    CHECK( lstrip(std::string("cba"), std::string("")) == std::string("cba") );
    CHECK( lstrip(std::string("ba"), std::string("b")) == std::string("a") );
    CHECK( lstrip(std::string("a"), std::string("b")) == std::string("a") );
    CHECK( lstrip(std::string("ba"), std::string("ba")) == std::string("") );
    CHECK( lstrip(std::string("a"), std::string("ba")) == std::string("") );
    CHECK( lstrip(std::string(""), std::string("ba")) == std::string("") );
    CHECK( lstrip(std::string(""), std::string("a")) == std::string("") );
    CHECK( lstrip(std::string(""), std::string("")) == std::string("") );




}
