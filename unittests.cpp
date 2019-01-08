#ifdef WITH_CATCH
#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "contrib/catch.hpp"
#elif defined(WITH_DOCTEST)
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "contrib/doctest.h"
#define SECTION SUBCASE
#else
#error define either WITH_CATCH or WITH_DOCTEST
#endif

#include "stringconvert.h"
#include <vector>

TEST_CASE("stringconvert") {
    SECTION("strlen") {
        // tests for the string::z::length function, which basically is a templated version
        // of the libc  strlen / wcslen functions.

        const char emptystr[1] = { 0 };
        const char len1str[2] = { 0x41, 0 };
        const short wemptystr[1] = { 0 };
        const short wlen1str[2] = { 0x41, 0 };

        CHECK( string::z::length(emptystr) == 0 );
        CHECK( string::z::length(len1str) == 1 );
        CHECK( string::z::length(wemptystr) == 0 );
        CHECK( string::z::length(wlen1str) == 1 );

        // check negatives
        CHECK( string::z::length(emptystr) != 1 );
        CHECK_FALSE( string::z::length(emptystr) != 0 );
        CHECK_FALSE( string::z::length(emptystr) == 1 );
    }
    SECTION("str2str") {
        uint32_t u32pts[] = { 1, 0x7f, 0x80, 0x100, 0x7ff, 0x800, 0xffff, 0x10000, 0x10ffff, 0 };
        int u32len = sizeof(u32pts)/sizeof(*u32pts) - 1;


        uint16_t u16pts[] = { 0x0001, 0x007f, 0x0080, 0x0100, 0x07ff, 0x0800, 0xffff, 0xd800, 0xdc00, 0xdbff, 0xdfff, 0 };
        int u16len = sizeof(u16pts)/sizeof(*u16pts) - 1;

        uint8_t u8pts[] = { 0x01, 0x7f, 0xc2, 0x80, 0xc4, 0x80, 0xdf, 0xbf, 0xe0, 0xa0, 0x80, 0xef, 0xbf, 0xbf, 0xf0, 0x90, 0x80, 0x80, 0xf4, 0x8f, 0xbf, 0xbf, 0 };
        int u8len = sizeof(u8pts)/sizeof(*u8pts) - 1;

        SECTION("types") {
            // char -> char
            CHECK( string::convert<char>(std::string("abcd")) == std::string("abcd") );
            // wchar -> wchar
            CHECK( string::convert<wchar_t>(std::wstring(L"abcd")) == std::wstring(L"abcd") );

            // wchar -> char
            CHECK( string::convert<char>(std::wstring(L"abcd")) == std::string("abcd") );

            // char -> wchar
            CHECK( string::convert<wchar_t>(std::string("abcd")) == std::wstring(L"abcd") );

            // check negatives
            CHECK( string::convert<wchar_t>(std::string("abcd")) != std::wstring(L"dcba") );
            CHECK_FALSE( string::convert<wchar_t>(std::string("abcd")) != std::wstring(L"abcd") );
            CHECK_FALSE( string::convert<wchar_t>(std::string("abcd")) == std::wstring(L"dcba") );
        }
        SECTION("badecodes") {
            std::vector<std::vector<uint8_t> > bad8list = {
                { 0x80 },  // code can never start with 0x80
                { 0xbf },  // code can never start with 0xbf
                { 0xf8 },  // f8, fc  are invalid high codes
                { 0xfc },  // f8, fc  are invalid high codes
                { 0xed, 0xa0, 0x80 },       //   d800            1101 1000 0000 0000             1101 100000 000000   ed a0 80
                { 0xed, 0xaf, 0xbf },       //   dbff            1101 1011 1111 1111             1101 101111 111111   ed af bf
                { 0xed, 0xb0, 0x80 },       //   dc00            1101 1100 0000 0000             1101 110000 000000   ed b0 80
                { 0xed, 0xbf, 0xbf },       //   dfff            1101 1111 1111 1111             1101 111111 111111   ed bf bf
                { 0xf4, 0x90, 0x80, 0x80 }, // 110000  0001 0001 0000 0000 0000 0000  (000)100 010000 000000 000000   f4 90 80 80
                { 0xc0, 0x80, },            //   NUL should be encoded as a single NUL
                { 0xe0, 0x80, 0x80, },      //   NUL should be encoded as a single NUL
                { 0xf0, 0x80, 0x80, 0x80 }, //   NUL should be encoded as a single NUL
                { 0xc0, 0xc0  },            //  after start pattern, there must be trailer bytes
                { 0xc0, 0x00  },            //  after start pattern, there must be trailer bytes
                { 0xe0, 0x80, 0x00 }
            };
            std::vector<std::vector<uint16_t> > bad16list = {
                { 0xd800, 0xd800 },   // only valid extcode: d8xx + dcyy
                { 0xdc00, 0xdc00 },
                { 0xdc00, 0xd800 },
                { 0xd800, 0x0000 },
                { 0xdc00, 0x0000 },
            };
            std::vector<std::vector<uint32_t> > bad32list = {
                { 0xd800 },
                { 0xdbff },
                { 0xdc00 },
                { 0xdfff },
                { 0x110000 },  // too large
                { 0x200000 },
            };
            auto mkstring = [](auto v) { return std::basic_string<typename decltype(v)::value_type>(v.begin(), v.end()); };

            // note: problem
            //   convert<char>("bad utf8 string") -> contents of string are not checked.
            for (int i=0 ; i<bad8list.size() ; i++) {
                INFO("utf8 " << i);
                CHECK( string::convert<wchar_t>(mkstring(bad8list[i])) == L"" );
            }
            for (int i=0 ; i<bad16list.size() ; i++) {
                INFO("utf16 " << i);
                CHECK( string::convert<char>(mkstring(bad16list[i])) == "" );
            }
            for (int i=0 ; i<bad32list.size() ; i++) {
                INFO("utf32 " << i);
                CHECK( string::convert<char>(mkstring(bad32list[i])) == "" );
            }

        }
        SECTION("codepoints") {
            auto u32 = std::basic_string<uint32_t>(u32pts, u32pts+u32len);
            auto i32 = std::basic_string<int32_t>(u32pts, u32pts+u32len);
            auto w = std::basic_string<wchar_t>(u32pts, u32pts+u32len);
            auto u16 = std::basic_string<uint16_t>(u16pts, u16pts+u16len);
            auto i16 = std::basic_string<int16_t>(u16pts, u16pts+u16len);
            auto s16 = std::basic_string<short>(u16pts, u16pts+u16len);
            auto u8 = std::basic_string<uint8_t>(u8pts, u8pts+u8len);
            auto i8 = std::basic_string<int8_t>(u8pts, u8pts+u8len);
            auto c8 = std::basic_string<char>(u8pts, u8pts+u8len);


            CHECK( string::convert<uint8_t>(u8) == u8 );
            CHECK( string::convert<uint8_t>(u16) == u8 );
            CHECK( string::convert<uint8_t>(u32) == u8 );
            CHECK( string::convert<uint16_t>(u8) == u16 );
            CHECK( string::convert<uint16_t>(u16) == u16 );
            CHECK( string::convert<uint16_t>(u32) == u16 );
            CHECK( string::convert<uint32_t>(u8) == u32 );
            CHECK( string::convert<uint32_t>(u16) == u32 );
            CHECK( string::convert<uint32_t>(u32) == u32 );

            CHECK( string::convert<uint8_t>(i8) == u8 );
            CHECK( string::convert<uint8_t>(i16) == u8 );
            CHECK( string::convert<uint8_t>(i32) == u8 );
            CHECK( string::convert<uint16_t>(i8) == u16 );
            CHECK( string::convert<uint16_t>(i16) == u16 );
            CHECK( string::convert<uint16_t>(i32) == u16 );
            CHECK( string::convert<uint32_t>(i8) == u32 );
            CHECK( string::convert<uint32_t>(i16) == u32 );
            CHECK( string::convert<uint32_t>(i32) == u32 );

            CHECK( string::convert<uint8_t>(c8) == u8 );
            CHECK( string::convert<uint8_t>(s16) == u8 );
            CHECK( string::convert<uint8_t>(w) == u8 );
            CHECK( string::convert<uint16_t>(c8) == u16 );
            CHECK( string::convert<uint16_t>(s16) == u16 );
            CHECK( string::convert<uint16_t>(w) == u16 );
            CHECK( string::convert<uint32_t>(c8) == u32 );
            CHECK( string::convert<uint32_t>(s16) == u32 );
            CHECK( string::convert<uint32_t>(w) == u32 );


            CHECK( string::convert<int8_t>(u8) == i8 );
            CHECK( string::convert<int8_t>(u16) == i8 );
            CHECK( string::convert<int8_t>(u32) == i8 );
            CHECK( string::convert<int16_t>(u8) == i16 );
            CHECK( string::convert<int16_t>(u16) == i16 );
            CHECK( string::convert<int16_t>(u32) == i16 );
            CHECK( string::convert<int32_t>(u8) == i32 );
            CHECK( string::convert<int32_t>(u16) == i32 );
            CHECK( string::convert<int32_t>(u32) == i32 );

            CHECK( string::convert<int8_t>(i8) == i8 );
            CHECK( string::convert<int8_t>(i16) == i8 );
            CHECK( string::convert<int8_t>(i32) == i8 );
            CHECK( string::convert<int16_t>(i8) == i16 );
            CHECK( string::convert<int16_t>(i16) == i16 );
            CHECK( string::convert<int16_t>(i32) == i16 );
            CHECK( string::convert<int32_t>(i8) == i32 );
            CHECK( string::convert<int32_t>(i16) == i32 );
            CHECK( string::convert<int32_t>(i32) == i32 );

            CHECK( string::convert<int8_t>(c8) == i8 );
            CHECK( string::convert<int8_t>(s16) == i8 );
            CHECK( string::convert<int8_t>(w) == i8 );
            CHECK( string::convert<int16_t>(c8) == i16 );
            CHECK( string::convert<int16_t>(s16) == i16 );
            CHECK( string::convert<int16_t>(w) == i16 );
            CHECK( string::convert<int32_t>(c8) == i32 );
            CHECK( string::convert<int32_t>(s16) == i32 );
            CHECK( string::convert<int32_t>(w) == i32 );

            // ---

            CHECK( string::convert<uint8_t>(u8pts) == u8 );
            CHECK( string::convert<uint8_t>(u16pts) == u8 );
            CHECK( string::convert<uint8_t>(u32pts) == u8 );
            CHECK( string::convert<uint16_t>(u8pts) == u16 );
            CHECK( string::convert<uint16_t>(u16pts) == u16 );
            CHECK( string::convert<uint16_t>(u32pts) == u16 );
            CHECK( string::convert<uint32_t>(u8pts) == u32 );
            CHECK( string::convert<uint32_t>(u16pts) == u32 );
            CHECK( string::convert<uint32_t>(u32pts) == u32 );

            CHECK( string::convert<int8_t>(u8pts) == i8 );
            CHECK( string::convert<int8_t>(u16pts) == i8 );
            CHECK( string::convert<int8_t>(u32pts) == i8 );
            CHECK( string::convert<int16_t>(u8pts) == i16 );
            CHECK( string::convert<int16_t>(u16pts) == i16 );
            CHECK( string::convert<int16_t>(u32pts) == i16 );
            CHECK( string::convert<int32_t>(u8pts) == i32 );
            CHECK( string::convert<int32_t>(u16pts) == i32 );
            CHECK( string::convert<int32_t>(u32pts) == i32 );
        }
    }
}

#include "formatter.h"


struct mytype { };
inline std::ostream& operator<<(std::ostream&os, const mytype& s) { return os << "MYTYPE"; }

TEST_CASE("formatter") {
    SECTION("singleformats") {
        CHECK( stringformat("%%") == "%" );
        CHECK( stringformat("%b", std::string("abc")) == "61 62 63  abc" );
        CHECK( stringformat("%i", 123) == "123" );
        CHECK( stringformat("%d", 123) == "123" );
        CHECK( stringformat("%u", 123) == "123" );
        CHECK( stringformat("%o", 123) == "173" );
        CHECK( stringformat("%x", 123) == "7b" );
        CHECK( stringformat("%X", 123) == "7B" );
        CHECK( stringformat("%f", 123.45) == "123.450000" );
        CHECK( stringformat("%F", 123.45) == "123.450000" );
        CHECK( stringformat("%g", 123.4567) == "123.457" );
        CHECK( stringformat("%G", 123.4567) == "123.457" );
        CHECK( stringformat("%g", 1234567.89) == "1.23457e+06" );
        CHECK( stringformat("%G", 1234567.89) == "1.23457E+06" );

        CHECK( stringformat("%a", 123.45) == "0x1.edccccccccccdp+6" );
        CHECK( stringformat("%A", 123.45) == "0X1.EDCCCCCCCCCCDP+6" );

        CHECK( stringformat("%e", 123.45) == "1.234500e+02" );
        CHECK( stringformat("%E", 123.45) == "1.234500E+02" );
        CHECK( stringformat("%c", 122) == "z" );
        CHECK( stringformat("%s", "a-c-string") == "a-c-string" );
        CHECK( stringformat("%s", std::string("a-std-string")) == "a-std-string" );

        // check negatives
        CHECK( stringformat("%i", 123) != "321" );
        CHECK_FALSE( stringformat("%i", 123) != "123" );
        CHECK_FALSE( stringformat("%i", 123) == "321" );
    }
    SECTION("hexints") {
        CHECK( stringformat("%02x", uint8_t(123)) == "7b" );
        CHECK( stringformat("%04x", uint8_t(123)) == "007b" );
        CHECK( stringformat("%04x", uint16_t(12345)) == "3039" );
        CHECK( stringformat("%08x", uint32_t(123456789)) == "075bcd15" );
        CHECK( stringformat("%02x", int(123)) == "7b" );
        CHECK( stringformat("%04x", int(123)) == "007b" );
        CHECK( stringformat("%04x", int(12345)) == "3039" );
        CHECK( stringformat("%08x", int(123456789)) == "075bcd15" );

        CHECK( stringformat("%02x", unsigned(123)) == "7b" );
        CHECK( stringformat("%04x", unsigned(123)) == "007b" );
        CHECK( stringformat("%04x", unsigned(12345)) == "3039" );
        CHECK( stringformat("%08x", unsigned(123456789)) == "075bcd15" );

    }
    SECTION("decints") {
        CHECK( stringformat("%02d", uint8_t(123)) == "123" );
        CHECK( stringformat("%04d", uint8_t(123)) == "0123" );
        CHECK( stringformat("%06d", uint16_t(12345)) == "012345" );
        CHECK( stringformat("%010d", uint32_t(123456789)) == "0123456789" );
        CHECK( stringformat("%02d", int(123)) == "123" );
        CHECK( stringformat("%04d", int(123)) == "0123" );
        CHECK( stringformat("%06d", int(12345)) == "012345" );
        CHECK( stringformat("%010d", int(123456789)) == "0123456789" );

        CHECK( stringformat("%02d", unsigned(123)) == "123" );
        CHECK( stringformat("%04d", unsigned(123)) == "0123" );
        CHECK( stringformat("%06d", unsigned(12345)) == "012345" );
        CHECK( stringformat("%010d", unsigned(123456789)) == "0123456789" );

    }
    SECTION("strings") {
        CHECK( stringformat("%s", "abcd") == "abcd" );
        CHECK( stringformat("%8s", "abcd") == "    abcd" );
        CHECK( stringformat("%-8s", "abcd") == "abcd    " );
        CHECK( stringformat("%3.3s", "abcd") == "abcd" );     // ... todo: should return "abc"
    }
    SECTION("hexdumps") {
        CHECK( stringformat("%b", "abc") == "" );      // hexdump of c-string does not work.

        CHECK( stringformat("%b", std::string("abc")) == "61 62 63  abc" );
        CHECK( stringformat("%0b", std::string("abc")) == "616263  abc" );

        CHECK( stringformat("%-b", std::string("abc")) == "61 62 63" );
        CHECK( stringformat("%-0b", std::string("abc")) == "616263" );

        // hexdump other types
        CHECK( stringformat("%b", std::vector<uint8_t>{1,2,3}) == "01 02 03  ..." );
        CHECK( stringformat("%b", std::vector<uint16_t>{1,2,3}) == "0001 0002 0003  ......" );

        // printing a hexdumper
        CHECK( stringformat("%-b", Hex::dumper("abc", 3)) == "61 62 63" );
    }
    SECTION("inttypes") {
        CHECK( stringformat("%I64d", 1) == "1" );
        CHECK( stringformat("%lld", 1) == "1" );
        CHECK( stringformat("%hd", 1) == "1" );
        CHECK( stringformat("%ld", 1) == "1" );
        CHECK( stringformat("%d", 1) == "1" );
        CHECK( stringformat("%d", 1L) == "1" );
        CHECK( stringformat("%d", 1U) == "1" );
        CHECK( stringformat("%d", size_t(1)) == "1" );
        CHECK( stringformat("%d", uint64_t(1)) == "1" );
        CHECK( stringformat("%d", uint32_t(1)) == "1" );
        CHECK( stringformat("%d", uint64_t(-1)) == "18446744073709551615" );
        CHECK( stringformat("%d", uint32_t(-1)) == "4294967295" );
    }
    SECTION("floats") {
        CHECK( stringformat("%.4f", 123.4) == "123.4000" );
        CHECK( stringformat("%.4f", 123.456789) == "123.4568" );
    }
    SECTION("padding") {
        CHECK( stringformat("%-3d", 1) == "1  " );
        CHECK( stringformat("%3d", 1) == "  1" );
        CHECK( stringformat("%+d", 1) == "+1" );
        CHECK( stringformat("%+d", -1) == "-1" );


        CHECK( stringformat("%-3s", "") == "   " );
        CHECK( stringformat("%-3s", "a") == "a  " );
        CHECK( stringformat("%-3s", "ab") == "ab " );
        CHECK( stringformat("%-3s", "abc") == "abc" );
        CHECK( stringformat("%-3s", "abcd") == "abcd" );

        CHECK( stringformat("%3s", "") == "   " );
        CHECK( stringformat("%3s", "a") == "  a" );
        CHECK( stringformat("%3s", "ab") == " ab" );
        CHECK( stringformat("%3s", "abc") == "abc" );
        CHECK( stringformat("%3s", "abcd") == "abcd" );


        //CHECK( stringformat("%3.3s", "abc") == "abc" );
        //CHECK( stringformat("%3.3s", "abcd") == "abc" );
        //CHECK( stringformat("%-3.3s", "abcd") == "abc" );

    }
    SECTION("argcount") {
        CHECK( stringformat("%d %d %d", 1LL, 1LL, 1LL) == "1 1 1" );

        CHECK( stringformat("") == "" );

        // too many args
        CHECK_THROWS( stringformat("test", 1) );
        CHECK_THROWS( stringformat("%d %d", 1, 2, 3) );

        // too few args
        CHECK_THROWS( stringformat("%d") );
        CHECK_THROWS( stringformat("%d %d", 1) );

        // invalid format char
        CHECK_THROWS( stringformat("%y", 1) );
    }
    SECTION("strings") {
        CHECK( stringformat("%s", "c-string") == "c-string" );
        CHECK( stringformat("%s", std::string("std-string")) == "std-string" );
        CHECK( stringformat("%s", std::wstring(L"std-string")) == "std-string" );

        // test with large unicode character
        std::basic_string<uint8_t> u8{0xf4, 0x8f, 0xbf, 0xbf};
        std::basic_string<uint16_t> u16{0xdbff, 0xdfff};
        std::basic_string<uint32_t> u32{0x10ffff};

        std::string c8 = string::convert<char>(u8);


        CHECK( stringformat("%s", u8) == c8 );
        CHECK( stringformat("%s", u16) == c8 );
        CHECK( stringformat("%s", u32) == c8 );
    }
    SECTION("vectors") {
        CHECK( stringformat("%s", std::vector<int>{1,2,3,4}) == "1 2 3 4" );
        CHECK( stringformat("%,s", std::vector<int>{1,2,3,4}) == "1,2,3,4" );

        CHECK( stringformat("%s", std::vector<double>{1,2,3,4}) == "1 2 3 4" );
        CHECK( stringformat("%s", std::vector<uint8_t>{1,2,3,4}) == "1 2 3 4" );
    }
    SECTION("arrays") {
        CHECK( stringformat("%s", std::array<int, 4>{{1,2,3,4}}) == "1 2 3 4" );
        CHECK( stringformat("%,s", std::array<int, 4>{{1,2,3,4}}) == "1,2,3,4" );

        CHECK( stringformat("%s", std::array<double, 4>{{1,2,3,4}}) == "1 2 3 4" );
        CHECK( stringformat("%s", std::array<uint8_t, 4>{{1,2,3,4}}) == "1 2 3 4" );
    }
    SECTION("custom") {
        CHECK( stringformat("%s", mytype()) == "MYTYPE" );
    }
    SECTION("fromcpython") {
        CHECK( stringformat("%.1d", 1) == "1" );
        //CHECK( stringformat("%.100d", 1) == "0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001" );
        CHECK( stringformat("%0100d", 1) == "0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001" );
        CHECK( stringformat("%f", 1.0) == "1.000000" );
        CHECK( stringformat("%x", 10) == "a" );
        CHECK( stringformat("%x", 100000000000) == "174876e800" );
        CHECK( stringformat("%o", 10) == "12" );
        CHECK( stringformat("%o", 100000000000) == "1351035564000" );
        CHECK( stringformat("%d", 10) == "10" );
        CHECK( stringformat("%d", 100000000000) == "100000000000" );
        CHECK( stringformat("%d", 42) == "42" );
        CHECK( stringformat("%d", -42) == "-42" );
        CHECK( stringformat("%d", 42) == "42" );
        CHECK( stringformat("%d", -42) == "-42" );
        CHECK( stringformat("%d", 42.0) == "42" );
        CHECK( stringformat("%o", 0) == "0" );
        CHECK( stringformat("%o", 0) == "0" );
        CHECK( stringformat("%d", 0) == "0" );
        CHECK( stringformat("%d", 0) == "0" );
        CHECK( stringformat("%x", 0x42) == "42" );
        CHECK( stringformat("%x", 0x42) == "42" );
        CHECK( stringformat("%o", 042) == "42" );
        CHECK( stringformat("%o", 042) == "42" );
#ifdef SUPPORT_NEGATIVE_HEX_OCT
unittests.cpp:406: FAILED:	  CHECK( stringformat("%x", -0x42) == "-42" )	with expansion:	  "ffffffffffffffbe" == "-42"	
unittests.cpp:408: FAILED:	  CHECK( stringformat("%x", -0x42) == "-42" )	with expansion:	  "ffffffffffffffbe" == "-42"	
unittests.cpp:410: FAILED:	  CHECK( stringformat("%o", -042) == "-42" )	with expansion:	  "1777777777777777777736" == "-42"	
unittests.cpp:412: FAILED:	  CHECK( stringformat("%o", -042) == "-42" )	with expansion:	  "1777777777777777777736" == "-42"	
        CHECK( stringformat("%x", -0x42) == "-42" );
        CHECK( stringformat("%x", -0x42) == "-42" );
        CHECK( stringformat("%o", -042) == "-42" );
        CHECK( stringformat("%o", -042) == "-42" );
#endif
        CHECK( stringformat("%g", 1.1) == "1.1" );
    }
    SECTION("fromwxmac") {
        // tests taken from the 'wx for mac' library
        CHECK( stringformat("%+d", 123456) == "+123456" );
        CHECK( stringformat("%d", -123456) == "-123456" );
#ifdef SUPPORT_SPACE_FOR_PLUS
unittests.cpp:418: FAILED:	  CHECK( stringformat("% d", 123456) == " 123456" )	with expansion:	  "123456" == " 123456"	
        CHECK( stringformat("% d", 123456) == " 123456" );
#endif
        CHECK( stringformat("%10d", 123456) == "    123456" );
        CHECK( stringformat("%010d", 123456) == "0000123456" );
        CHECK( stringformat("%-10d", -123456) == "-123456   " );
        CHECK( stringformat("%X", 0xABCD) == "ABCD" );
#ifdef SUPPORT_POUND_SIGN
unittests.cpp:423: FAILED:	  CHECK( stringformat("%#X", 0xABCD) == "0XABCD" )	due to unexpected exception with message:	  unknown format char	
unittests.cpp:424: FAILED:	  CHECK( stringformat("%#x", 0xABCD) == "0xabcd" )	due to unexpected exception with message:	  unknown format char	
unittests.cpp:426: FAILED:	  CHECK( stringformat("%#o", 01234567) == "01234567" )	due to unexpected exception with message:	  unknown format char	


        CHECK( stringformat("%#X", 0xABCD) == "0XABCD" );
        CHECK( stringformat("%#x", 0xABCD) == "0xabcd" );
        CHECK( stringformat("%o", 01234567) == "1234567" );
        CHECK( stringformat("%#o", 01234567) == "01234567" );
#endif
#ifdef SUPPORT_POINTERS
unittests.cpp:427: FAILED:	  CHECK( stringformat("%p", (void*)0xABCDEF) == "00ABCDEF" )	with expansion:	  "0xabcdef" == "00ABCDEF"	
unittests.cpp:428: FAILED:	  CHECK( stringformat("%p", (void*)__null) == "00000000" )	with expansion:	  "0x0" == "00000000"	
unittests.cpp:429: FAILED:	  CHECK( stringformat("%p", (void*)0xABCDEFABCDEF) == "0000ABCDEFABCDEF" )	with expansion:	  "0xabcdefabcdef" == "0000ABCDEFABCDEF"	
unittests.cpp:430: FAILED:	  CHECK( stringformat("%p", (void*)__null) == "0000000000000000" )	with expansion:	  "0x0" == "0000000000000000"	
unittests.cpp:432: FAILED:	  CHECK( stringformat("%p", (void*)__null) == "(nil)" )	with expansion:	  "0x0" == "(nil)"	


        CHECK( stringformat("%p", (void*)0xABCDEF) == "00ABCDEF" );
        CHECK( stringformat("%p", (void*)NULL) == "00000000" );
        CHECK( stringformat("%p", (void*)0xABCDEFABCDEF) == "0000ABCDEFABCDEF" );
        CHECK( stringformat("%p", (void*)NULL) == "0000000000000000" );
        CHECK( stringformat("%p", (void*)0xABCDEF) == "0xabcdef" );
        CHECK( stringformat("%p", (void*)NULL) == "(nil)" );
#endif
        CHECK( stringformat("%e",2.342E+112) == "2.342000e+112" );
        CHECK( stringformat("%10.4e",-2.342E-112) == "-2.3420e-112" );
        CHECK( stringformat("%11.4e",-2.342E-112) == "-2.3420e-112" );
        CHECK( stringformat("%15.4e",-2.342E-112) == "   -2.3420e-112" );
        CHECK( stringformat("%G",-2.342E-02) == "-0.02342" );
        CHECK( stringformat("%G",3.1415e-116) == "3.1415E-116" );
        CHECK( stringformat("%016e", 3141.5e100) == "0003.141500e+103" );
        CHECK( stringformat("%16e", 3141.5e100) == "   3.141500e+103" );
        CHECK( stringformat("%-16e", 3141.5e100) == "3.141500e+103   " );
        CHECK( stringformat("%010.3e", 3141.5e100) == "3.142e+103" );
        CHECK( stringformat("%5f", 3.3) == "3.300000" );
        CHECK( stringformat("%5f", 3.0) == "3.000000" );
        CHECK( stringformat("%5f", .999999E-4) == "0.000100" );
        CHECK( stringformat("%5f", .99E-3) == "0.000990" );
        CHECK( stringformat("%5f", 3333.0) == "3333.000000" );
        CHECK( stringformat("%5g", 3.3) == "  3.3" );
        CHECK( stringformat("%5g", 3.0) == "    3" );
        CHECK( stringformat("%5g", .999999E-114) == "9.99999e-115" );
        CHECK( stringformat("%5g", .99E-3) == "0.00099" );
        CHECK( stringformat("%5g", 3333.0) == " 3333" );
        CHECK( stringformat("%5g", 0.01) == " 0.01" );
        //CHECK( stringformat("%5.g", 3.3) == "    3" );
        CHECK( stringformat("%5.g", 3.0) == "    3" );
        //CHECK( stringformat("%5.g", .999999E-114) == "1e-114" );
        CHECK( stringformat("%5.g", 1.0E-4) == "0.0001" );
        //CHECK( stringformat("%5.g", .99E-3) == "0.001" );
        //CHECK( stringformat("%5.g", 3333.0E100) == "3e+103" );
        CHECK( stringformat("%5.g", 0.01) == " 0.01" );
        CHECK( stringformat("%5.2g", 3.3) == "  3.3" );
        CHECK( stringformat("%5.2g", 3.0) == "    3" );
        CHECK( stringformat("%5.2g", .999999E-114) == "1e-114" );
        CHECK( stringformat("%5.2g", .99E-3) == "0.00099" );
        CHECK( stringformat("%5.2g", 3333.0E100) == "3.3e+103" );
        CHECK( stringformat("%5.2g", 0.01) == " 0.01" );
        CHECK( stringformat("%5s", "abc") == "  abc" );
        CHECK( stringformat("%5s", "a") == "    a" );
        CHECK( stringformat("%5s", "abcdefghi") == "abcdefghi" );
        CHECK( stringformat("%-5s", "abc") == "abc  " );
        CHECK( stringformat("%-5s", "abcdefghi") == "abcdefghi" );
#ifdef SUPPORT_STRING_TRUNCATION
        CHECK( stringformat("%.5s", "abcdefghi") == "abcde" );
#endif
#ifdef SUPPORT_VARIABLE_WIDTH
unittests.cpp:473: FAILED:	  CHECK( stringformat("%*.*f", 10, 1, 0.123) == "       0.1" )	due to unexpected exception with message:	  unknown format char	
unittests.cpp:474: FAILED:	  CHECK( stringformat("%*.*f", 10, 4, 0.123) == "    0.1230" )	due to unexpected exception with message:	  unknown format char	
unittests.cpp:475: FAILED:	  CHECK( stringformat("%*.*f", 3, 1, 0.123) == "0.1" )	due to unexpected exception with message:	  unknown format char	
unittests.cpp:476: FAILED:	  CHECK( stringformat("%%%.*f", 3, 0.0023456789) == "%0.002" )	due to unexpected exception with message:	  unknown format char	
unittests.cpp:477: FAILED:	  CHECK( stringformat("%*c", 8, 'a') == "       a" )	due to unexpected exception with message:	  unknown format char	
unittests.cpp:478: FAILED:	  CHECK( stringformat("%*s", 8, "four") == "    four" )	due to unexpected exception with message:	  unknown format char	
unittests.cpp:479: FAILED:	  CHECK( stringformat("%*s %*s", 8, "four", 6, "four") == "    four   four" )	due to unexpected exception with message:	  unknown format char	
unittests.cpp:490: FAILED:	  CHECK( stringformat("%4$.3f %1$i - test - %2$li %3$d", 123, 444444444, 555, -0.666) == "-0.666 123 - test - 444444444 555" )	due to unexpected exception with message:	  unknown format char	
        CHECK( stringformat("%*.*f", 10, 1, 0.123) == "       0.1" );
        CHECK( stringformat("%*.*f", 10, 4, 0.123) == "    0.1230" );
        CHECK( stringformat("%*.*f", 3, 1, 0.123) == "0.1" );
        CHECK( stringformat("%%%.*f", 3, 0.0023456789) == "%0.002" );
        CHECK( stringformat("%*c", 8, 'a') == "       a" );
        CHECK( stringformat("%*s", 8, "four") == "    four" );
        CHECK( stringformat("%*s %*s", 8, "four", 6, "four") == "    four   four" );
#endif
        CHECK( stringformat("%%") == "%" );
        CHECK( stringformat("%%%%%%") == "%%%" );
        CHECK( stringformat("%%%5s", "abc") == "%  abc" );
        CHECK( stringformat("%%%5s%%", "abc") == "%  abc%" );
        CHECK( stringformat("%lld", 123456789) == "123456789" );
        CHECK( stringformat("%lld", -123456789) == "-123456789" );
        CHECK( stringformat("%llu", 123456789) == "123456789" );
        CHECK( stringformat("%I64d", 123456789) == "123456789" );
        CHECK( stringformat("%I64x", 0x123456789abcdef) == "123456789abcdef" );
        CHECK( stringformat("%i %li - test - %d %.3f", 123, 444444444, 555, -0.666) == "123 444444444 - test - 555 -0.666" );
// todo: what does '$' mean?
//        CHECK( stringformat("%4$.3f %1$i - test - %2$li %3$d", 123, 444444444, 555, -0.666) == "-0.666 123 - test - 444444444 555" );
        CHECK( stringformat("unicode string: %ls %lc - ansi string: %hs %hc\n\n", L"unicode!!", L'W', "ansi!!", 'w') == "unicode string: unicode!! W - ansi string: ansi!! w\n\n" );

    }
    SECTION("reactos") {
        // unittests taken from the reactos printf test suite
        // - many commented out tests are in disaggreement with my format library
//CHECK( stringformat("%I", 1) ==  "I" );
#ifdef SUPPORT_POUND_SIGN
unittests.cpp:496: FAILED:	  CHECK( stringformat("%#04.8x", 1) == "0x00000001" )	due to unexpected exception with message:	  unknown format char	
unittests.cpp:497: FAILED:	  CHECK( stringformat("%#-08.2x", 1) == "0x01    " )	due to unexpected exception with message:	  unknown format char	
unittests.cpp:498: FAILED:	  CHECK( stringformat("%#.0x", 1) == "0x1" )	due to unexpected exception with message:	  unknown format char	
unittests.cpp:499: FAILED:	  CHECK( stringformat("%#08o", 1) == "00000001" )	due to unexpected exception with message:	  unknown format char	
unittests.cpp:500: FAILED:	  CHECK( stringformat("%#o", 1) == "01" )	due to unexpected exception with message:	  unknown format char	
CHECK( stringformat("%#04.8x", 1) == "0x00000001" );
CHECK( stringformat("%#-08.2x", 1) == "0x01    " );
CHECK( stringformat("%#.0x", 1) == "0x1" );
CHECK( stringformat("%#08o", 1) == "00000001" );
CHECK( stringformat("%#o", 1) == "01" );
#endif
wchar_t wide[] = { 'w','i','d','e',0};
// 'w'  is a microsoft specific format size specifier.
//'w'CHECK( stringformat("%ws",  wide) == "wide" );
//'w'CHECK( stringformat("%-10ws",  wide ) == "wide      " );
//'w'CHECK( stringformat("%10ws",  wide ) == "      wide" );
//CHECK( stringformat("%#+ -03whlls",  wide ) == "wide" );
//CHECK( stringformat("%w0s",  wide ) == "0s" );
//CHECK( stringformat("%w-s",  wide ) == "-s" );
CHECK( stringformat("%ls",  wide ) == "wide" );
CHECK( stringformat("%Ls",  "not wide" ) == "not wide" );
CHECK( stringformat("%3c", 'a') == "  a" );
CHECK( stringformat("%3d", 1234) == "1234" );
CHECK( stringformat("%-1d", 2) == "2" );
CHECK( stringformat("%2.4f", 8.6) == "8.6000" );
CHECK( stringformat("%0f", 0.6) == "0.600000" );
CHECK( stringformat("%.0f", 0.6) == "1" );
CHECK( stringformat("%2.4e", 8.6) == "8.6000e+00" );
#ifdef SUPPORT_SPACE_FOR_PLUS
CHECK( stringformat("% 2.4e", 8.6) == " 8.6000e+00" );
CHECK( stringformat("% 014.4e", 8.6) == " 008.6000e+00" );
CHECK( stringformat("% 2.4e", -8.6) == "-8.6000e+00" );
#endif
CHECK( stringformat("%+2.4e", 8.6) == "+8.6000e+00" );
CHECK( stringformat("%2.4g", 8.6) == "8.6" );
CHECK( stringformat("%-i", -1) == "-1" );
CHECK( stringformat("%-i", 1) == "1" );
CHECK( stringformat("%+i", 1) == "+1" );
CHECK( stringformat("%o", 10) == "12" );
//CHECK( stringformat("%s", 0) == "(null)" );
CHECK( stringformat("%s", "%%%%") == "%%%%" );

// ... 'u'  does not have a special meaning in formatter.h
//CHECK( stringformat("%u", -1) == "4294967295" );
//
//  these are all invalid format strings
//CHECK( stringformat("%w", -1) == "" );
//CHECK( stringformat("%h", -1) == "" );
//CHECK( stringformat("%z", -1) == "z" );
//CHECK( stringformat("%j", -1) == "j" );
//CHECK( stringformat("%F", -1) == "" );
//CHECK( stringformat("%N", -1) == "" );
//CHECK( stringformat("%H", -1) == "H" );

// returns unicode string with formatter.h
//CHECK( stringformat("x%cx",  0x100+'X') == "xXx" );

// 'h' does not truncate it's argument.
//CHECK( stringformat("%hx",  0x12345) == "2345" );
//CHECK( stringformat("%hhx",  0x123) == "123" );
//
//these are all unicode chars
//CHECK( stringformat("%c", 0x3031) == "1" );
//CHECK( stringformat("%hc", 0x3031) == "1" );
//CHECK( stringformat("%wc", 0x3031) == "?" );
//CHECK( stringformat("%lc", 0x3031) == "?" );
//CHECK( stringformat("%Lc", 0x3031) == "1" );
//CHECK( stringformat("%Ic", 0x3031) == "Ic" );
//CHECK( stringformat("%Iwc", 0x3031) == "Iwc" );
//CHECK( stringformat("%I32c", 0x3031) == "1" );
//CHECK( stringformat("%I64c", 0x3031) == "1" );
//CHECK( stringformat("%4c", 0x3031) == "   1" );
//CHECK( stringformat("%04c", 0x3031) == "0001" );
//CHECK( stringformat("%+4c", 0x3031) == "   1" );
CHECK( stringformat("%d", 1234567) == "1234567" );
CHECK( stringformat("%d", -1234567) == "-1234567" );
// 'h' does not truncate
//CHECK( stringformat("%hd", 1234567) == "-10617" );
CHECK( stringformat("%08d", 1234) == "00001234" );
//  our '0' adds 0 padding to the end.
//CHECK( stringformat("%-08d", 1234) == "1234    " );
//TODO  our '+' ends up at the wrong location.
//CHECK( stringformat("%+08d", 1234) == "+0001234" );
CHECK( stringformat("%+3d", 1234) == "+1234" );
CHECK( stringformat("%3.3d", 1234) == "1234" );
//??? CHECK( stringformat("%3.6d", 1234) == "001234" );
CHECK( stringformat("%8d", -1234) == "   -1234" );
//TODO  our '+' ends up at the wrong location.
//TODO: CHECK( stringformat("%08d", -1234) == "-0001234" );
CHECK( stringformat("%ld", -1234) == "-1234" );
//'w' CHECK( stringformat("%wd", -1234) == "-1234" );
// 'l' does not truncate
//CHECK( stringformat("%ld", -5149074030855LL) == "591757049" );
//CHECK( stringformat("%lld", -5149074030855LL) == "591757049" );
CHECK( stringformat("%I64d", -5149074030855LL) == "-5149074030855" );
//CHECK( stringformat("%Ld", -5149074030855LL) == "591757049" );
//'w' CHECK( stringformat("%lhwI64d", -5149074030855LL) == "-5149074030855" );
//'w' CHECK( stringformat("%I64hlwd", -5149074030855LL) == "-5149074030855" );
//'w' CHECK( stringformat("%hlwd", -5149074030855LL) == "32505" );
//'I' CHECK( stringformat("%Ild", -5149074030855LL) == "Ild" );
//'h' CHECK( stringformat("%hd", -5149074030855LL) == "32505" );
//'h' CHECK( stringformat("%hhd", -5149074030855LL) == "32505" );
//'h' CHECK( stringformat("%hI32hd", -5149074030855LL) == "32505" );
//'w' CHECK( stringformat("%wd", -5149074030855LL) == "591757049" );
CHECK( stringformat("%u", 1234) == "1234" );
//'u' CHECK( stringformat("%u", -1234) == "4294966062" );
//'u' CHECK( stringformat("%lu", -1234) == "4294966062" );
//'u' CHECK( stringformat("%llu", -1234) == "4294966062" );
//'u' CHECK( stringformat("%+u", 1234) == "1234" );
//' ' CHECK( stringformat("% u", 1234) == "1234" );
CHECK( stringformat("%x", 0x1234abcd) == "1234abcd" );
CHECK( stringformat("%X", 0x1234abcd) == "1234ABCD" );
//'#'CHECK( stringformat("%#x", 0x1234abcd) == "0x1234abcd" );
//'#'CHECK( stringformat("%#X", 0x1234abcd) == "0X1234ABCD" );
//notrunc CHECK( stringformat("%llx", 0x1234abcd5678ULL) == "abcd5678" );
CHECK( stringformat("%I64x", 0x1234abcd5678ULL) == "1234abcd5678" );
//CHECK( stringformat("%p", (void*)(ptrdiff_t)0x123abc) == "00123ABC" );
//'#'CHECK( stringformat("%#p", (void*)(ptrdiff_t)0x123abc) == "0X00123ABC" );
//'#'CHECK( stringformat("%#012p", (void*)(ptrdiff_t)0x123abc) == "  0X00123ABC" );
//p CHECK( stringformat("%9p", (void*)(ptrdiff_t)0x123abc) == " 00123ABC" );
//p CHECK( stringformat("%09p", (void*)(ptrdiff_t)0x123abc) == " 00123ABC" );
//p CHECK( stringformat("% 9p", (void*)(ptrdiff_t)0x123abc) == " 00123ABC" );
//p CHECK( stringformat("%-9p", (void*)(ptrdiff_t)0x123abc) == "00123ABC " );
//p CHECK( stringformat("%4p", (void*)(ptrdiff_t)0x123abc) == "00123ABC" );
//p CHECK( stringformat("%9.4p", (void*)(ptrdiff_t)0x123abc) == " 00123ABC" );
//CHECK( stringformat("%I64p", 0x123abc456789ULL) == "123ABC456789" );
//'h' CHECK( stringformat("%hp", 0x123abc) == "00003ABC" );
CHECK( stringformat("%o", 1234) == "2322" );
//CHECK( stringformat("%o", -1234) == "37777775456" );
CHECK( stringformat("%s", "test") == "test" );
//CHECK( stringformat("%S", L"test") == "test" );
CHECK( stringformat("%ls", L"test") == "test" );
//'w' CHECK( stringformat("%ws", L"test") == "test" );
CHECK( stringformat("%hs", "test") == "test" );
//CHECK( stringformat("%hS", "test") == "test" );
CHECK( stringformat("%7s", "test") == "   test" );
CHECK( stringformat("%07s", "test") == "000test" );
//CHECK( stringformat("%.3s", "test") == "tes" );
//CHECK( stringformat("%+7.3s", "test") == "    tes" );
//CHECK( stringformat("%+4.0s", "test") == "    " );
long double fpval = 1. / 3.;
//?? CHECK( stringformat("%f", fpval) == "-0.000000" );
//?? CHECK( stringformat("%lf", fpval) == "-0.000000" );
//?? CHECK( stringformat("%llf", fpval) == "-0.000000" );
//?? CHECK( stringformat("%Lf", fpval) == "-0.000000" );
CHECK( stringformat("%f", (double)fpval) == "0.333333" );
CHECK( stringformat("%f", (double)0.125) == "0.125000" );
CHECK( stringformat("%3.7f", (double)fpval) == "0.3333333" );
//CHECK( stringformat("%3.30f", (double)fpval) == "0.333333333333333310000000000000" );
//CHECK( stringformat("%3.60f", (double)fpval) == "0.333333333333333310000000000000000000000000000000000000000000" );
//CHECK( stringformat("%3.80f", (double)fpval) == "0.33333333333333331000000000000000000000000000000000000000000000000000000000000000" );
//notypeconversion CHECK( stringformat("%.9f", 0x7ff8000000000000ULL) == "1.#QNAN0000" );
CHECK( stringformat("%e", 33.54223) == "3.354223e+01" );
//noportableCHECK( stringformat("%e", NAN) == "1.#QNAN0e+000" );
//noportableCHECK( stringformat("%.9e", NAN) == "1.#QNAN0000e+000" );
//noportableCHECK( stringformat("%e", INFINITY ) == "1.#INF00e+000" );
//noportableCHECK( stringformat("%e", -INFINITY ) == "-1.#INF00e+000" );
//unknown CHECK( stringformat("%Z", 0) == "(null)" );
//'w' CHECK( stringformat("%lI64wQ", "test") == "Q" );

    }
}


#include "hexdumper.h"

TEST_CASE("hexdumper") {
    SECTION("hex") {
        std::stringstream buf;
        buf << Hex::hexstring << std::setfill(' ') << Hex::dumper("abcde", 5);
        CHECK( buf.str() == "61 62 63 64 65" );
    }
    SECTION("left") {
        std::stringstream buf;
        buf << Hex::singleline << std::left <<  Hex::dumper("abcde", 5);
        CHECK( buf.str() == "61 62 63 64 65" );
    }
    SECTION("right") {
        std::stringstream buf;
        buf << Hex::singleline << std::right <<  Hex::dumper("abcde", 5);
        CHECK( buf.str() == "abcde" );
    }
    SECTION("bin") {
        std::stringstream buf;
        buf << Hex::bin << std::left << Hex::dumper(std::vector<uint8_t>{123});

        CHECK( buf.str() == "01111011" );
    }
    SECTION("hexstring") {
        std::stringstream buf;
        buf << Hex::hexstring << Hex::dumper("abcde", 5);
        CHECK( buf.str() == "6162636465" );
    }
    SECTION("ascstring") {
        std::stringstream buf;
        buf << Hex::ascstring << Hex::dumper("abcde\r\n", 7);
        CHECK( buf.str() == "abcde.." );
    }

    SECTION("ascstring") {
        std::vector<uint8_t> bv;
        std::string asc;
        for (int i=0 ; i<256 ; i++) {
            bv.push_back(i);
            asc.push_back( (i<32) || (i>=0x7f) ? '.' : char(i) );
        }

        std::stringstream buf;
        buf << Hex::ascstring << Hex::dumper(bv);

        CHECK( buf.str() == asc );
    }
    SECTION("singleline") {
        std::stringstream buf;
        buf << Hex::singleline <<  Hex::dumper("abcde", 5);
        CHECK( buf.str() == "61 62 63 64 65  abcde" );
    }
    SECTION("multiline") {
        std::stringstream buf;
        buf << Hex::multiline <<  Hex::dumper("0123456789abcdefghijklmnopq", 27);
        CHECK( buf.str() == 
"30 31 32 33 34 35 36 37 38 39 61 62 63 64 65 66  0123456789abcdef\n"
"67 68 69 6a 6b 6c 6d 6e 6f 70 71                 ghijklmnopq\n");
    }
    SECTION("offset") {
        std::stringstream buf;
        buf << Hex::offset(0xa000) << Hex::multiline <<  Hex::dumper("0123456789abcdefghijklmnopq", 27);
        CHECK( buf.str() == 
"0000a000: 30 31 32 33 34 35 36 37 38 39 61 62 63 64 65 66  0123456789abcdef\n"
"0000a010: 67 68 69 6a 6b 6c 6d 6e 6f 70 71                 ghijklmnopq\n");
    }
    SECTION("step") {
        // this test currently fails
        std::stringstream buf;
        buf << Hex::offset(0xa000) << Hex::step(3) << std::setw(2) <<  Hex::dumper("0123456789abcdefghijklmnopq", 27);
        CHECK( buf.str() == 
"0000a000: 30 31  01\n"
"0000a003: 33 34  34\n"
"0000a006: 36 37  67\n"
"0000a009: 39 61  9a\n"
"0000a00c: 63 64  cd\n"
"0000a00f: 66 67  fg\n"
"0000a012: 69 6a  ij\n"
"0000a015: 6c 6d  lm\n"
"0000a018: 6f 70  op\n");
    }
    SECTION("values") {
        std::stringstream buf;
        SECTION("byte") {
            buf << Hex::singleline << std::left << Hex::dumper(std::vector<uint8_t>{0, 0x55, 127, 128, 0xAA, 255});
            CHECK( buf.str() == "00 55 7f 80 aa ff" );
        }
        SECTION("char") {
            buf << Hex::singleline << std::left << Hex::dumper(std::vector<char>{0, 0x55, 127, (char)-128, (char)-0x56, (char)-1});
            CHECK( buf.str() == "00 55 7f 80 aa ff" );
        }
        SECTION("hword") {
            buf << Hex::singleline << std::left << Hex::dumper(std::vector<uint16_t>{0, 127, 128, 255, 0x5555, 0x7fff, 0x8000, 0xAAAA, 0xFFFF});
            CHECK( buf.str() == "0000 007f 0080 00ff 5555 7fff 8000 aaaa ffff");
        }
        SECTION("short") {
            buf << Hex::singleline << std::left << Hex::dumper(std::vector<int16_t>{0, 127, 128, 255, 0x5555, 0x7fff, -0x8000, -0x5556, -1});
            CHECK( buf.str() == "0000 007f 0080 00ff 5555 7fff 8000 aaaa ffff");
        }
        SECTION("dword") {
            buf << Hex::singleline << std::left << Hex::dumper(std::vector<uint32_t>{0, 127, 128, 255, 0x7fff, 0x8000, 0xFFFF, 0x55555555, 0x7FFFFFFF, 0x80000000, 0xAAAAAAAA, 0xFFFFFFFF});
            CHECK( buf.str() == "00000000 0000007f 00000080 000000ff 00007fff 00008000 0000ffff 55555555 7fffffff 80000000 aaaaaaaa ffffffff" );
        }
        SECTION("long") {
            buf << Hex::singleline << std::left << Hex::dumper(std::vector<int32_t>{0, 127, 128, 255, 0x7fff, 0x8000, 0xFFFF, 0x55555555, 0x7FFFFFFF, -0x7fffffff-1, -0x55555556, -1});
            CHECK( buf.str() == "00000000 0000007f 00000080 000000ff 00007fff 00008000 0000ffff 55555555 7fffffff 80000000 aaaaaaaa ffffffff" );
        }
        SECTION("longlong") {
            buf << Hex::singleline << std::left << Hex::dumper(std::vector<int64_t>{0, 127, 128, 255, 0x7fff, 0x8000, 0xFFFF, 0x55555555, 0x7FFFFFFF, -0x7fffffff-1, -0x55555556, -1, 0x5555555555555555, -0x5555555555555556, 0x7FFFFFFFFFFFFFFF, -0x7FFFFFFFFFFFFFFF-1});
            CHECK( buf.str() == "0000000000000000 000000000000007f 0000000000000080 00000000000000ff 0000000000007fff 0000000000008000 000000000000ffff 0000000055555555 000000007fffffff ffffffff80000000 ffffffffaaaaaaaa ffffffffffffffff 5555555555555555 aaaaaaaaaaaaaaaa 7fffffffffffffff 8000000000000000" );
        }
        SECTION("ulonglong") {
            buf << Hex::singleline << std::left << Hex::dumper(std::vector<uint64_t>{0, 127, 128, 255, 0x7fff, 0x8000, 0xFFFF, 0x55555555, 0x7FFFFFFF, 0x80000000, 0xAAAAAAAA, 0xFFFFFFFF, 0x5555555555555555, 0xAAAAAAAAAAAAAAAA, 0x7FFFFFFFFFFFFFFF, 0x8000000000000000, 0xFFFFFFFFFFFFFFFF});
            CHECK( buf.str() == "0000000000000000 000000000000007f 0000000000000080 00000000000000ff 0000000000007fff 0000000000008000 000000000000ffff 0000000055555555 000000007fffffff 0000000080000000 00000000aaaaaaaa 00000000ffffffff 5555555555555555 aaaaaaaaaaaaaaaa 7fffffffffffffff 8000000000000000 ffffffffffffffff" );
        }
    }
}
#include "stringlibrary.h"

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
                auto pos = res.second-t.str.begin();  // in seperate variable for doctest.h
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
}

#include "argparse.h"
TEST_CASE("argparse") {
    SECTION("test") {
        const char*argv[] = {
            "pgmname",               // not counted in the option list
            "-a", "123",             // mask 0x0001, counted in nargs,  value checked
            "--bigword",             // mask 0x0200, counted in nbig
            "-b123",                 // mask 0x0002, counted in nargs
            "-pear", "0x1234",       // mask 0x0004, counted in nargs,  value checked
            "-vvv",                  // mask 0x0400, counted in nargs, multiplicity checked
            "-pTEST",                // mask 0x0008, counted in nargs, value checked
            "--apple=test",          // mask 0x0010, counted in nargs, value checked
            "--equal==test",         // mask 0x2000, counted in nargs, value checked
            "--bigword",             // mask 0x0200, counted in nbig
            "--long", "999",         // mask 0x0020, counted in nargs, value checked
            "firstfile",             // mask 0x0080, counted in nfiles, value checked
            "secondfile",            // mask 0x0100, counted in nfiles, value checked,
            "-",                     // mask 0x0040, counted in nstdin 
            "--",                    // mask 0x1000, counted in nrends
            "moreargs", "-v", "-"    // mask 0x0800, counted in nextra
        };
        int argc = sizeof(argv)/sizeof(*argv);
        int nfiles = 0;
        int nextra = 0;
        int argmask = 0;
        int nargs = 0;
        int nbig = 0;
        int nstdin = 0;
        int nrends = 0;
        for (auto& arg : ArgParser(argc, argv))
            if (nrends) {
                nextra ++;
                argmask |= 0x0800;
            }
            else switch(arg.option())
            {
                case 'a':
                    CHECK( arg.getint() == 123 );
                    argmask |= 0x0001;
                    nargs ++;
                    break;

                case 'b':
                    CHECK( arg.getint() == 123 );
                    argmask |= 0x0002;
                    nargs ++;
                    break;
                case 'p':
                    if (arg.match("-pear")) {
                        CHECK( arg.getint() == 0x1234 );
                        argmask |= 0x0004;
                        nargs ++;
                    }
                    else {
                        CHECK( std::string(arg.getstr()) == "TEST" );
                        argmask |= 0x0008;
                        nargs ++;
                    }
                    break;
                case 'v':
                    CHECK( arg.count() == 3 );
                    argmask |= 0x0400;
                    nargs ++;
                    break;
                case '-':
                    if (arg.match("--big")) {
                        nbig++;
                        argmask |= 0x0200;
                    }
                    else if (arg.match("--bigword")) {
                        // --big should match before --bigword
                        CHECK( false );
                    }
                    else if (arg.match("--unused")) {
                        CHECK( false );
                    }
                    else if (arg.match("--apple")) {
                        CHECK( std::string(arg.getstr()) == "test" );
                        argmask |= 0x0010;
                        nargs ++;
                    }
                    else if (arg.match("--equal")) {
                        CHECK( std::string(arg.getstr()) == "=test" );
                        argmask |= 0x2000;
                        nargs ++;
                    }
                    else if (arg.match("--long")) {
                        CHECK( arg.getint() == 999 );
                        argmask |= 0x0020;
                        nargs ++;
                    }
                    else if (arg.optionterminator()) {
                        nrends ++;
                        argmask |= 0x1000;
                    }
                    else {
                        INFO( "unexpected long option" );
                        CHECK( false );
                    }
                    break;
                case 0:
                    CHECK( std::string(arg.getstr()) == "-" );
                    nstdin ++;
                    argmask |= 0x0040;
                    break;
                case -1:
                    switch(nfiles++)
                    {
                        case 0:
                            CHECK( std::string(arg.getstr()) == "firstfile" );
                            argmask |= 0x0080;
                            break;
                        case 1:
                            CHECK( std::string(arg.getstr()) == "secondfile" );
                            argmask |= 0x0100;
                            break;
                        default:
                            INFO( "expected only two non options args" );
                            CHECK( false );
                    }
                    break;
                default:
                    INFO( "unexpected option" );
                    CHECK( false );
            }
        CHECK( nstdin == 1 );
        CHECK( nrends == 1 );
        CHECK( nfiles == 2 );
        CHECK( nextra == 3 );
        CHECK( nargs == 8 );
        CHECK( nbig == 2 );
        CHECK( argmask == 0x3FFF );
    }
}
