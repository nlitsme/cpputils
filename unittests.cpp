
#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "contrib/catch.hpp"

#include "stringconvert.h"

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
}


#include "hexdumper.h"

TEST_CASE("hexdumper") {
    SECTION("hex") {
        std::stringstream buf;
        buf << hex::hexstring << std::setfill(' ') << hex::dumper("abcde", 5);
        CHECK( buf.str() == "61 62 63 64 65" );
    }
    SECTION("left") {
        std::stringstream buf;
        buf << hex::singleline << std::left <<  hex::dumper("abcde", 5);
        CHECK( buf.str() == "61 62 63 64 65" );
    }
    SECTION("right") {
        std::stringstream buf;
        buf << hex::singleline << std::right <<  hex::dumper("abcde", 5);
        CHECK( buf.str() == "abcde" );
    }
    SECTION("bin") {
        std::stringstream buf;
        buf << hex::bin << std::left << hex::dumper(std::vector<uint8_t>{123});

        CHECK( buf.str() == "01111011" );
    }
    SECTION("hexstring") {
        std::stringstream buf;
        buf << hex::hexstring << hex::dumper("abcde", 5);
        CHECK( buf.str() == "6162636465" );
    }
    SECTION("ascstring") {
        std::stringstream buf;
        buf << hex::ascstring << hex::dumper("abcde\r\n", 7);
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
        buf << hex::ascstring << hex::dumper(bv);

        CHECK( buf.str() == asc );
    }
    SECTION("singleline") {
        std::stringstream buf;
        buf << hex::singleline <<  hex::dumper("abcde", 5);
        CHECK( buf.str() == "61 62 63 64 65  abcde" );
    }
    SECTION("multiline") {
        std::stringstream buf;
        buf << hex::multiline <<  hex::dumper("0123456789abcdefghijklmnopq", 27);
        CHECK( buf.str() == 
"30 31 32 33 34 35 36 37 38 39 61 62 63 64 65 66  0123456789abcdef\n"
"67 68 69 6a 6b 6c 6d 6e 6f 70 71                 ghijklmnopq\n");
    }
    SECTION("offset") {
        std::stringstream buf;
        buf << hex::offset(0xa000) << hex::multiline <<  hex::dumper("0123456789abcdefghijklmnopq", 27);
        CHECK( buf.str() == 
"0000a000: 30 31 32 33 34 35 36 37 38 39 61 62 63 64 65 66  0123456789abcdef\n"
"0000a010: 67 68 69 6a 6b 6c 6d 6e 6f 70 71                 ghijklmnopq\n");
    }
    SECTION("step") {
        // this test currently fails
        std::stringstream buf;
        buf << hex::offset(0xa000) << hex::step(3) << std::setw(2) <<  hex::dumper("0123456789abcdefghijklmnopq", 27);
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
            buf << hex::singleline << std::left << hex::dumper(std::vector<uint8_t>{0, 0x55, 127, 128, 0xAA, 255});
            CHECK( buf.str() == "00 55 7f 80 aa ff" );
        }
        SECTION("char") {
            buf << hex::singleline << std::left << hex::dumper(std::vector<char>{0, 0x55, 127, -128, -0x56, -1});
            CHECK( buf.str() == "00 55 7f 80 aa ff" );
        }
        SECTION("hword") {
            buf << hex::singleline << std::left << hex::dumper(std::vector<uint16_t>{0, 127, 128, 255, 0x5555, 0x7fff, 0x8000, 0xAAAA, 0xFFFF});
            CHECK( buf.str() == "0000 007f 0080 00ff 5555 7fff 8000 aaaa ffff");
        }
        SECTION("short") {
            buf << hex::singleline << std::left << hex::dumper(std::vector<int16_t>{0, 127, 128, 255, 0x5555, 0x7fff, -0x8000, -0x5556, -1});
            CHECK( buf.str() == "0000 007f 0080 00ff 5555 7fff 8000 aaaa ffff");
        }
        SECTION("dword") {
            buf << hex::singleline << std::left << hex::dumper(std::vector<uint32_t>{0, 127, 128, 255, 0x7fff, 0x8000, 0xFFFF, 0x55555555, 0x7FFFFFFF, 0x80000000, 0xAAAAAAAA, 0xFFFFFFFF});
            CHECK( buf.str() == "00000000 0000007f 00000080 000000ff 00007fff 00008000 0000ffff 55555555 7fffffff 80000000 aaaaaaaa ffffffff" );
        }
        SECTION("long") {
            buf << hex::singleline << std::left << hex::dumper(std::vector<int32_t>{0, 127, 128, 255, 0x7fff, 0x8000, 0xFFFF, 0x55555555, 0x7FFFFFFF, -0x7fffffff-1, -0x55555556, -1});
            CHECK( buf.str() == "00000000 0000007f 00000080 000000ff 00007fff 00008000 0000ffff 55555555 7fffffff 80000000 aaaaaaaa ffffffff" );
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
            CHECK( parseunsigned(empty, 0) == std::make_pair(0ULL, empty.cbegin()) );
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
            };

            for (auto& t : tests) {
                int delta = (t.endpos==-1) ? t.str.size() : t.endpos;

                const auto pend = t.str.cbegin() + delta;
                const auto *cend = t.str.c_str() + delta;

                INFO( "test str=" << t.str << " base=" << t.base << " delta=" << delta );
                CHECK( parseunsigned(t.str, t.base) == std::make_pair(t.value, pend) );
                CHECK( parseunsigned(t.str.c_str(), t.base) == std::make_pair(t.value, cend) );
            }
        }
    }

    SECTION("parsesigned") {
        SECTION("positivenumbers") {
            struct testvalues {
                int base;
                std::string str;
                int64_t value;

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
            };

            for (const auto& t : tests) {
                int delta = (t.endpos==-1) ? t.str.size() : t.endpos;

                const auto pend = t.str.cbegin() + delta;
                const auto *cend = t.str.c_str() + delta;

                INFO( "test str=" << t.str << " base=" << t.base << " delta=" << delta );
                CHECK( parsesigned(t.str, t.base) == std::make_pair(t.value, pend) );
                CHECK( parsesigned(t.str.c_str(), t.base) == std::make_pair(t.value, cend) );
            }
        }
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
            };

            for (const auto& t : tests) {
                int delta = (t.endpos==-1) ? t.str.size() : t.endpos;

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
        const char*argv[] = { "pgmname", "-a", "123", "-b123", "-pear", "0x1234", "-vvv", "-pTEST", "--apple=test", "--long", "999", "firstfile", "secondfile", "-" };
        int argc = sizeof(argv)/sizeof(*argv);
        int n = 0;
        int argmask = 0;
        int nargs = 0;
        bool foundstdin = false;
        for (auto& arg : ArgParser(argc, argv))
            switch(arg.option())
            {
                case 'a':
                    CHECK( arg.getint() == 123 );
                    argmask |= 1;
                    nargs ++;
                    break;

                case 'b':
                    CHECK( arg.getint() == 123 );
                    argmask |= 2;
                    nargs ++;
                    break;
                case 'p':
                    if (arg.match("-pear")) {
                        CHECK( arg.getint() == 0x1234 );
                        argmask |= 4;
                        nargs ++;
                    }
                    else {
                        CHECK( std::string(arg.getstr()) == "TEST" );
                        argmask |= 8;
                        nargs ++;
                    }
                    break;
                case 'v':
                    CHECK( arg.count() == 3 );
                    break;
                case '-':
                    if (arg.match("--unused")) {
                        CHECK( false );
                    }
                    else if (arg.match("--apple")) {
                        CHECK( std::string(arg.getstr()) == "test" );
                        argmask |= 16;
                        nargs ++;
                    }
                    else if (arg.match("--long")) {
                        CHECK( arg.getint() == 999 );
                        argmask |= 32;
                        nargs ++;
                    }
                    else {
                        INFO( "unexpected long option" );
                        CHECK( false );
                    }
                    break;
                case 0:
                    foundstdin = true;
                    argmask |= 64;
                    nargs ++;
                    break;
                case -1:
                    switch(n++)
                    {
                        case 0:
                            CHECK( std::string(arg.getstr()) == "firstfile" );
                            argmask |= 128;
                            nargs ++;
                            break;
                        case 1:
                            CHECK( std::string(arg.getstr()) == "secondfile" );
                            argmask |= 256;
                            nargs ++;
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
        CHECK( foundstdin == true );
        CHECK( n == 2 );
        CHECK( nargs == 9 );
        CHECK( argmask == 0x1FF );
    }
}
