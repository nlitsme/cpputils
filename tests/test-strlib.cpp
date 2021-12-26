#include "unittestframework.h"

#include "stringlibrary.h"
#include "stringlibrary.h"

#include <string>
#include <vector>
#include <tuple>

TEST_CASE("stringlibrary") {
    SECTION("charptr") {
        SECTION("compare") {
            CHECK( stringcompare("abcd", "abcd") == 0 );
            CHECK( stringcompare("abcd", "abc") > 0 );
            CHECK( stringcompare("abcd", "abcde") < 0 );
            CHECK( stringcompare("aaaa", "zzzz") < 0 );
            CHECK( stringcompare("zzzz", "aaaa") > 0 );
            CHECK( stringcompare("", "") == 0 );
            CHECK( stringcompare("", "a") < 0 );
            CHECK( stringcompare("a", "") > 0 );

            CHECK( stringcompare(L"abcd", L"abcd") == 0 );
            CHECK( stringcompare(L"abcd", L"abc") > 0 );
            CHECK( stringcompare(L"abcd", L"abcde") < 0 );
            CHECK( stringcompare(L"aaaa", L"zzzz") < 0 );
            CHECK( stringcompare(L"zzzz", L"aaaa") > 0 );
            CHECK( stringcompare(L"", L"") == 0 );
            CHECK( stringcompare(L"", L"a") < 0 );
            CHECK( stringcompare(L"a", L"") > 0 );
        }
        SECTION("icompare") {
            CHECK( stringicompare("ABCD", "abcd") == 0 );
            CHECK( stringicompare("abcd", "abcd") == 0 );
            CHECK( stringicompare("ABCD", "abc") > 0 );
            CHECK( stringicompare("ABCD", "abcde") < 0 );
            CHECK( stringicompare("AAAA", "zzzz") < 0 );
            CHECK( stringicompare("ZZZZ", "aaaa") > 0 );
            CHECK( stringicompare("", "") == 0 );
            CHECK( stringicompare("", "A") < 0 );
            CHECK( stringicompare("A", "") > 0 );

            CHECK( stringicompare(L"ABCD", L"abcd") == 0 );
            CHECK( stringicompare(L"ABCD", L"abc") > 0 );
            CHECK( stringicompare(L"ABCD", L"abcde") < 0 );
            CHECK( stringicompare(L"AAAA", L"zzzz") < 0 );
            CHECK( stringicompare(L"ZZZZ", L"aaaa") > 0 );
            CHECK( stringicompare(L"", L"") == 0 );
            CHECK( stringicompare(L"", L"a") < 0 );
            CHECK( stringicompare(L"a", L"") > 0 );
        }
    }
    SECTION("std::string") {

        SECTION("icompare") {
            CHECK( stringicompare(std::string("ABCD"), std::string("abcd")) == 0 );
            CHECK( stringicompare(std::string("abcd"), std::string("abcd")) == 0 );
            CHECK( stringicompare(std::string("ABCD"), std::string("abc")) > 0 );
            CHECK( stringicompare(std::string("ABCD"), std::string("abcde")) < 0 );
            CHECK( stringicompare(std::string("AAAA"), std::string("zzzz")) < 0 );
            CHECK( stringicompare(std::string("ZZZZ"), std::string("aaaa")) > 0 );
            CHECK( stringicompare(std::string(""), std::string("")) == 0 );
            CHECK( stringicompare(std::string(""), std::string("A")) < 0 );
            CHECK( stringicompare(std::string("A"), std::string("")) > 0 );

            CHECK( stringicompare(std::basic_string<wchar_t>(L"ABCD"), std::basic_string<wchar_t>(L"abcd")) == 0 );
            CHECK( stringicompare(std::basic_string<wchar_t>(L"ABCD"), std::basic_string<wchar_t>(L"abc")) > 0 );
            CHECK( stringicompare(std::basic_string<wchar_t>(L"ABCD"), std::basic_string<wchar_t>(L"abcde")) < 0 );
            CHECK( stringicompare(std::basic_string<wchar_t>(L"AAAA"), std::basic_string<wchar_t>(L"zzzz")) < 0 );
            CHECK( stringicompare(std::basic_string<wchar_t>(L"ZZZZ"), std::basic_string<wchar_t>(L"aaaa")) > 0 );
            CHECK( stringicompare(std::basic_string<wchar_t>(L""), std::basic_string<wchar_t>(L"")) == 0 );
            CHECK( stringicompare(std::basic_string<wchar_t>(L""), std::basic_string<wchar_t>(L"a")) < 0 );
            CHECK( stringicompare(std::basic_string<wchar_t>(L"a"), std::basic_string<wchar_t>(L"")) > 0 );
        }
    }
    SECTION("copy") {
        SECTION("char") {
            char buf[8];
            CHECK( stringcopy(buf, "abcd") == buf+4 );
            CHECK( memcmp(buf, "abcd", 4) == 0 );
        }
        SECTION("wchar") {
            wchar_t buf[8];
            CHECK( stringcopy(buf, L"abcd") == buf+4 );
            CHECK( memcmp(buf, L"abcd", 4*sizeof(wchar_t)) == 0 );
        }
    }
    SECTION("parseunsigned") {
        SECTION("empty") {
            std::string empty;
            CHECK( parseunsigned(empty, 0) == std::make_pair(uint64_t(0), empty.cbegin()) );
        }
        SECTION("conversions") {
            struct testvalues {
                int base;
                std::string str;
                uint64_t value;

                int endpos;
            };
            std::vector<testvalues> tests= {
                { 0, "12345678", 12345678,   -1 },
                { 0, "1234567812345678", 1234567812345678LL,   -1 },
                { 0, "01234567", 01234567,   -1 },
                { 0, "0123456712345671234567", 0123456712345671234567LL,   -1 },
                { 0, "0x12345678", 0x12345678,   -1 },
                { 0, "0x123456789abcdef", 0x123456789abcdef,   -1 },
                { 0, "0x123456789ABCDEF", 0x123456789abcdef,   -1 },
                { 0, "0b010101010101010101", 87381,   -1 },

                { 10, "12345678", 12345678,   -1 },
                { 10, "1234567812345678", 1234567812345678LL,   -1 },
                { 8, "1234567", 01234567,   -1 },
                { 8, "123456712345671234567", 0123456712345671234567LL,   -1 },
                { 16, "12345678", 0x12345678,   -1 },
                { 16, "123456789abcdef", 0x123456789abcdef,   -1 },
                { 2, "010101010101010101", 87381,   -1 },

                // check that non digits cause parsing to stop at the right pos.
                { 0, "12345678,test", 12345678,   8 },
                { 0, "12345678=test", 12345678,   8 },
                { 0, "12345678|test", 12345678,   8 },
                { 0, "01234567,test", 01234567,   8 },
                { 0, "0x12345678,test", 0x12345678,   10 },
                { 0, "0b010101010101010101,test", 87381,   20 },

                { 10, "12345678,test", 12345678,   8 },
                { 8, "1234567,test", 01234567,   7 },
                { 16, "12345678,test", 0x12345678,   8 },
                { 2, "010101010101010101,test", 87381,   18 },

                // non octal digits are invalid
                { 0, "012345678", 01234567,   8 },
                { 8, "012345678", 01234567,   8 },


                { 0, "0x12345678:", 0x12345678,   -2 },
                { 10, "0x12345678:", 0,   1 },
                { 0, ":0:", 0,   0 },
                { 0, ":1:", 0,   0 },
                { 0, "1234:", 1234,   4 },
                { 0, "1234:5678", 1234,   4 },
                { 0, "1234:", 1234,   -2 },
                { 0, "1", 1,   -1 },
                { 0, "0", 0,   -1 },
                { 0, "", 0,   0 },

                { 0, "0:0x123:16", 0,   1 },
                { 0, "1:0x123:16", 1,   1 },
                { 0, "0x123:16", 0x123,   5 },

                // a valid hex number.
                { 16, "0b", 11,   2 },

                // these are all valid decimal nrs followed by a letter.
                { 10, "0b", 0,   1 },
                { 10, "0x", 0,   1 },
                { 10, "0y", 0,   1 },

                // these should all be invalid numbers
                { 0, "0b", 0,   0 },
                { 0, "0x", 0,   0 },
                { 0, "0y", 0,   0 },
            };

            for (const auto& t : tests) {
                int delta = (t.endpos<0) ? t.endpos+t.str.size()+1 : t.endpos;

                const auto pend = t.str.cbegin() + delta;
                const auto *cend = t.str.c_str() + delta;

                auto res = parseunsigned(t.str.begin(), t.str.end(), t.base);
                auto pos = res.second-t.str.begin();  // in separate variable for doctest.h
                INFO( "test str=" << t.str << " base=" << t.base << " delta=" << delta << " -> " << res.first << ", " << pos);
                CHECK( parseunsigned(t.str, t.base) == std::make_pair(t.value, pend) );
                CHECK( parseunsigned(t.str.c_str(), t.base) == std::make_pair(t.value, cend) );

                CHECK( parsesigned(t.str, t.base) == std::make_pair((int64_t)t.value, pend) );
                CHECK( parsesigned(t.str.c_str(), t.base) == std::make_pair((int64_t)t.value, cend) );

            }
        }
    }

    SECTION("parsesigned") {
        SECTION("negativenumbers") {
            struct testvalues {
                int base;
                std::string str;
                int64_t value;

                int endpos;
            };
            std::vector<testvalues> tests= {
                { 0, "-12345678", -12345678,   -1 },
                { 0, "-1234567812345678", -1234567812345678LL,   -1 },
                { 0, "-01234567", -01234567,   -1 },
                { 0, "-0123456712345671234567", -0123456712345671234567LL,   -1 },
                { 0, "-0x12345678", -0x12345678,   -1 },
                { 0, "-0x123456789abcdef", -0x123456789abcdef,   -1 },
                { 0, "-0b010101010101010101", -87381,   -1 },

                { 10, "-12345678", -12345678,   -1 },
                { 10, "-1234567812345678", -1234567812345678LL,   -1 },
                { 8, "-1234567", -01234567,   -1 },
                { 8, "-123456712345671234567", -0123456712345671234567LL,   -1 },
                { 16, "-12345678", -0x12345678,   -1 },
                { 16, "-123456789abcdef", -0x123456789abcdef,   -1 },
                { 2, "-010101010101010101", -87381,   -1 },

                // check that non digits cause parsing to stop at the right pos.
                { 0, "-12345678,test", -12345678,   8+1 },
                { 0, "-01234567,test", -01234567,   8+1 },
                { 0, "-0x12345678,test", -0x12345678,   10+1 },
                { 0, "-0b010101010101010101,test", -87381,   20+1 },

                { 10, "-12345678,test", -12345678,   8+1 },
                { 8, "-1234567,test", -01234567,   7+1 },
                { 16, "-12345678,test", -0x12345678,   8+1 },
                { 2, "-010101010101010101,test", -87381,   18+1 },

                // non octal digits are invalid
                { 0, "-012345678", -01234567,   8+1 },
                { 8, "-012345678", -01234567,   8+1 },

                { 0, "-1:0x123:16", -1,   2 },
                { 0, "-0x123:16", -0x123,   6 },
            };

            for (const auto& t : tests) {
                int delta = (t.endpos<0) ? t.endpos+t.str.size()+1 : t.endpos;

                const auto pend = t.str.cbegin() + delta;
                const auto *cend = t.str.c_str() + delta;

                INFO( "test str=" << t.str << " base=" << t.base << " delta=" << delta );
                CHECK( parsesigned(t.str, t.base) == std::make_pair(t.value, pend) );
                CHECK( parsesigned(t.str.c_str(), t.base) == std::make_pair(t.value, cend) );
            }
        }
    }
    SECTION("hex2bin") {
        CHECK( hex2binary<std::vector<uint8_t>>("") == std::vector<uint8_t>{ } );
        //CHECK( hex2binary<std::vector<uint8_t>>("0") == std::vector<uint8_t>{ 0x00 } );
        CHECK( hex2binary<std::vector<uint8_t>>("00") == std::vector<uint8_t>{ 0x00 } );
        //CHECK( hex2binary<std::vector<uint8_t>>("000") == std::vector<uint8_t>{ 0x00, 0x00 } );
        CHECK( hex2binary<std::vector<uint8_t>>("00ff") == std::vector<uint8_t>{ 0x00, 0xff } );
        //CHECK( hex2binary<std::vector<uint8_t>>("0ff") == std::vector<uint8_t>{ 0x00, 0xff } );
    }
    SECTION("splitter") {
        SECTION("stdstring") {
            using namespace std::string_literals;

            std::vector<std::tuple<std::string, std::string, std::vector<std::string>>> testcases = {
                { "abc/def"s,     "/"s, { "abc"s, "def"s } },
                { "abc/def/klm"s, "/"s, { "abc"s, "def"s, "klm"s } },
//              { "abc/"s,        "/"s, { "abc"s, ""s, } },
//              { "/"s,           "/"s, { ""s, ""s, } },
                { ""s,            "/"s, { } },
                { "abc"s,         "/"s, { "abc"s } },
                { "ditis, een,test"s, ", "s, { "ditis"s, "een"s, "test"s } },
            };

            for (auto& tcase : testcases) {
                auto& result = std::get<2>(tcase);
                auto first = result.begin();
                auto last = result.end();
#ifdef USE_CATCH
                INFO("testing " << ::Catch::Detail::stringify(tcase));
#endif
                for (auto item : stringsplitter(std::get<0>(tcase), std::get<1>(tcase))) {
                    REQUIRE(first != last);
                    CHECK(item == *first++);
                }
                CHECK(first==last);
            }
        }
        SECTION("strref") {
            std::string str = "control a1 01 0100 0";

            const std::string& arg = str;
            int i = 0;
            for (auto s : stringsplitter<std::string_view>(arg, " ")) {
                switch(i++)
                {
                    case 0 : CHECK( s == "control" ); break;
                    case 1 : CHECK( s == "a1" ); break;
                    case 2 : CHECK( s == "01" ); break;
                    case 3 : CHECK( s == "0100" ); break;
                    case 4 : CHECK( s == "0" ); break;
                    case 5:
                             CHECK( false );
                }
            }
            CHECK( i == 5 );
        }
        SECTION("strview2") {
            std::string str = "control a1 01 0100 0";
            std::string_view view(str);

            int i = 0;
            for (auto s : stringsplitter<std::string_view>(view, " ")) {
                switch(i++)
                {
                    case 0 : CHECK( s == "control" ); break;
                    case 1 : CHECK( s == "a1" ); break;
                    case 2 : CHECK( s == "01" ); break;
                    case 3 : CHECK( s == "0100" ); break;
                    case 4 : CHECK( s == "0" ); break;
                    case 5:
                             CHECK( false );
                }
            }
            CHECK( i == 5 );
        }

    }
}

