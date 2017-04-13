
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
        CHECK( stringformat("%s", std::array<int, 4>{1,2,3,4}) == "1 2 3 4" );
        CHECK( stringformat("%,s", std::array<int, 4>{1,2,3,4}) == "1,2,3,4" );

        CHECK( stringformat("%s", std::array<double, 4>{1,2,3,4}) == "1 2 3 4" );
        CHECK( stringformat("%s", std::array<uint8_t, 4>{1,2,3,4}) == "1 2 3 4" );
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
