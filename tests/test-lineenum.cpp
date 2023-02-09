#include "unittestframework.h"

#include <cpputils/string-lineenum.h>
#include <cpputils/string-lineenum.h>

TEST_CASE("lineenum") {
    std::string text = "first\nsecond\nthird";
    int i=0;
    for (auto part : lineenumerator(text)) {
        switch(i++)
        {
            case 0: CHECK(part == "first"); break;
            case 1: CHECK(part == "second"); break;
            case 2: CHECK(part == "third"); break;
        }
    }
    CHECK( i==3 );  // --> 2 : fail to return last section.
}
TEST_CASE("lineenum_empty") {
    std::string text = "";
    int i=0;
    // verify that an empty string does have no lines.
    for (auto part : lineenumerator(text))
    {
        CHECK_FALSE(i++==0);
        CHECK_FALSE(part.empty());
    }
    CHECK(i==0);
}
TEST_CASE("lineenum_nolf") {
    std::string text = "test";
    int i=0;
    // verify that an string without lf is handled as one line.
    for (auto part : lineenumerator(text)) {
        CHECK(i++==0);
        CHECK( part == "test" );
    }
    CHECK(i==1);  // --> 0 : fail to return last section.
}
TEST_CASE("lineenum_lf") {
    std::string text = "test\n";
    int i=0;
    // verify that an string with an lf is handled as one line.
    for (auto part : lineenumerator(text)) {
        CHECK(i++==0);
        CHECK( part == "test" );
    }
    CHECK(i==1);
}
TEST_CASE("lineenum_onlylf") {
    std::string text = "\n";
    int i=0;
    // verify that an string with only an lf is handled as one empty line.
    for (auto part : lineenumerator(text)) {
        CHECK(i++==0);
        CHECK( part == "" );
    }
    CHECK(i==1);
}
TEST_CASE("lineenum_lflf") {
    std::string text = "\n\n";
    int i=0;
    // verify that an string with only an lf is handled as one empty line.
    for (auto part : lineenumerator(text)) {
        CHECK(i++<2);
        CHECK( part == "" );
    }
    CHECK(i==2);
}
// test\n\n
// \ntest
// \n\ntest
// \ntest\n

TEST_CASE("lineenumref") {
    std::string text = "first\nsecond\nthird";
    int i=0;
    for (const auto& part : lineenumerator(text)) {
        switch(i++)
        {
            case 0: CHECK(part == "first"); break;
            case 1: CHECK(part == "second"); break;
            case 2: CHECK(part == "third"); break;
        }
    }
}
#if 0
TEST_CASE("lineenumw") {
    std::wstring text = L"first\nsecond\nthird";
    int i=0;
    for (auto part : lineenumerator(text)) {
        switch(i++)
        {
            case 0: CHECK(part == L"first"); break;
            case 1: CHECK(part == L"second"); break;
            case 2: CHECK(part == L"third"); break;
        }
    }
}

TEST_CASE("lineenumwref") {
    std::wstring text = L"first\nsecond\nthird";
    int i=0;
    for (const auto& part : lineenumerator(text)) {
        switch(i++)
        {
            case 0: CHECK(part == L"first"); break;
            case 1: CHECK(part == L"second"); break;
            case 2: CHECK(part == L"third"); break;
        }
    }
}
TEST_CASE("lineenumvector") {
    std::vector<uint8_t> text = { 'a', '\n', 'b', '\n', 'c' };
    int i=0;
    for (auto part : lineenumerator(text)) {
        switch(i++)
        {
            case 0: CHECK(part == "a"); break;
            case 1: CHECK(part == "b"); break;
            case 2: CHECK(part == "c"); break;
        }
    }
}
#endif

