#include "unittestframework.h"

#include "stringconvert.h"
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
        SECTION("badcodes") {
            std::vector<std::vector<uint8_t> > bad8list = {
                { 0x80 },                   // code can never start with 0x80
                { 0xbf },                   // code can never start with 0xbf
                { 0xf8 },                   // f8, fc  are invalid high codes
                { 0xfc },                   // f8, fc  are invalid high codes
                { 0xed, 0xa0, 0x80 },       //   d800            1101 1000 0000 0000             1101 100000 000000   ed a0 80
                { 0xed, 0xaf, 0xbf },       //   dbff            1101 1011 1111 1111             1101 101111 111111   ed af bf
                { 0xed, 0xb0, 0x80 },       //   dc00            1101 1100 0000 0000             1101 110000 000000   ed b0 80
                { 0xed, 0xbf, 0xbf },       //   dfff            1101 1111 1111 1111             1101 111111 111111   ed bf bf
                { 0xf4, 0x90, 0x80, 0x80 }, // 110000  0001 0001 0000 0000 0000 0000  (000)100 010000 000000 000000   f4 90 80 80

// ... note: 'oldcv' would succeed here, because trailing NULs in strings were removed.
// todo         { 0xc0, 0x80, },            //?  NUL should be encoded as a single NUL
// todo         { 0xe0, 0x80, 0x80, },      //?  NUL should be encoded as a single NUL
// todo         { 0xf0, 0x80, 0x80, 0x80 }, //?  NUL should be encoded as a single NUL
                { 0xc0, 0xc0  },            //  after start pattern, there must be trailer bytes
// todo         { 0xc0, 0x00  },            //? after start pattern, there must be trailer bytes
// todo         { 0xe0, 0x80, 0x00 },       //?
            };
            std::vector<std::vector<uint16_t> > bad16list = {
                { 0xd800, 0xd800 },   // only valid extcode: d8xx + dcyy
                { 0xdc00, 0xdc00 },   //
                { 0xdc00, 0xd800 },   //
// todo         { 0xd800, 0x0000 },   //?  -- todo: verify that we are not expecting a surrogate value.
                { 0xdc00, 0x0000 },   //
            };
            std::vector<std::vector<uint32_t> > bad32list = {
                { 0xd800 },    // utf16 surrugate  value is an invalid code point.
                { 0xdbff },    //
                { 0xdc00 },    //
                { 0xdfff },    //
                { 0x110000 },  // code point too large
                { 0x200000 },  //
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
            auto w = (sizeof(wchar_t)==2)
                ? std::basic_string<wchar_t>(u16pts, u16pts+u16len)
                : std::basic_string<wchar_t>(u32pts, u32pts+u32len);
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
#if 1
            CHECK( string::convert<uint8_t>(u8pts, u8pts+u8len) == u8 );
            CHECK( string::convert<uint8_t>(u16pts, u16pts+u16len) == u8 );
            CHECK( string::convert<uint8_t>(u32pts, u32pts+u32len) == u8 );
            CHECK( string::convert<uint16_t>(u8pts, u8pts+u8len) == u16 );
            CHECK( string::convert<uint16_t>(u16pts, u16pts+u16len) == u16 );
            CHECK( string::convert<uint16_t>(u32pts, u32pts+u32len) == u16 );
            CHECK( string::convert<uint32_t>(u8pts, u8pts+u8len) == u32 );
            CHECK( string::convert<uint32_t>(u16pts, u16pts+u16len) == u32 );
            CHECK( string::convert<uint32_t>(u32pts, u32pts+u32len) == u32 );

            CHECK( string::convert<int8_t>(u8pts, u8pts+u8len) == i8 );
            CHECK( string::convert<int8_t>(u16pts, u16pts+u16len) == i8 );
            CHECK( string::convert<int8_t>(u32pts, u32pts+u32len) == i8 );
            CHECK( string::convert<int16_t>(u8pts, u8pts+u8len) == i16 );
            CHECK( string::convert<int16_t>(u16pts, u16pts+u16len) == i16 );
            CHECK( string::convert<int16_t>(u32pts, u32pts+u32len) == i16 );
            CHECK( string::convert<int32_t>(u8pts, u8pts+u8len) == i32 );
            CHECK( string::convert<int32_t>(u16pts, u16pts+u16len) == i32 );
            CHECK( string::convert<int32_t>(u32pts, u32pts+u32len) == i32 );
#else // old string conversion lib
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
#endif
        }
    }
}

