#include "unittestframework.h"

#include "formatter.h"


struct mytype { };
inline std::ostream& operator<<(std::ostream&os, const mytype& s) { return os << "MYTYPE"; }

struct Unprintable { };

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
#ifdef _WIN32
        CHECK( stringformat("%a", 123.45) == "0x1.edcccdp+6" );
        CHECK( stringformat("%A", 123.45) == "0X1.EDCCCDP+6" );
#else
        CHECK( stringformat("%a", 123.45) == "0x1.edccccccccccdp+6" );
        CHECK( stringformat("%A", 123.45) == "0X1.EDCCCCCCCCCCDP+6" );
#endif
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
    SECTION("charcv") {
        CHECK( stringformat("%c", char(122)) == "z" );
        CHECK( stringformat("%c", wchar_t(122)) == "z" );
        CHECK( stringformat("%c", wchar_t(0x20ac)) == "\xE2\x82\xAC" );
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
        CHECK( stringformat("%b", "abc") == "abc" );      // hexdump of c-string does not work.

        CHECK( stringformat("%b", std::string("abc")) == "61 62 63  abc" );
        CHECK( stringformat("%0b", std::string("abc")) == "616263  abc" );

        CHECK( stringformat("%-b", std::string("abc")) == "61 62 63" );
        CHECK( stringformat("%-0b", std::string("abc")) == "616263" );

        // hexdump other types
        CHECK( stringformat("%b", std::vector<uint8_t>{1,2,3}) == "01 02 03  ..." );
        CHECK( stringformat("%b", std::vector<uint16_t>{1,2,3}) == "0001 0002 0003  ......" );

        CHECK( stringformat("%-b", std::vector<uint8_t>{1,2,3}) == "01 02 03" );
        CHECK( stringformat("%-b", std::vector<uint16_t>{1,2,3}) == "0001 0002 0003" );

        CHECK( stringformat("%s", std::vector<uint8_t>{1,2,3}) == "1 2 3" );
        CHECK( stringformat("%s", std::vector<uint16_t>{1,2,3}) == "1 2 3" );

        CHECK( stringformat("%b", std::vector<uint8_t>{0x10,0x20,0x30}) == "10 20 30  . 0" );
        CHECK( stringformat("%b", std::vector<uint16_t>{0x10,0x20,0x30}) == "0010 0020 0030  .. .0." );

        CHECK( stringformat("%-b", std::vector<uint8_t>{0x10,0x20,0x30}) == "10 20 30" );
        CHECK( stringformat("%-b", std::vector<uint16_t>{0x10,0x20,0x30}) == "0010 0020 0030" );

        CHECK( stringformat("%s", std::vector<uint8_t>{0, 0x10,0x20,0x30, 0x7f,0x80,0xff}) == "0 16 32 48 127 128 255" );
        CHECK( stringformat("%s", std::vector<uint16_t>{0, 0x10,0x20,0x30,0xff,0xffff}) == "0 16 32 48 255 65535" );

        // printing a hexdumper
        CHECK( stringformat("%-b", Hex::dumper("abc\x01\x02", 5)) == "61 62 63 01 02" );
        CHECK( stringformat("%+b", Hex::dumper("abc\x01\x02", 5)) == "abc.." );
        CHECK( stringformat("% b", Hex::dumper("abc\x01\x02", 5)) == "61 62 63 01 02  abc.." );
        CHECK( stringformat("%0b", Hex::dumper("abc\x01\x02", 5)) == "6162630102  abc.." );

        CHECK_THROWS( stringformat("%0-b", Hex::dumper("abc\x01\x02", 5)) );
        CHECK_THROWS( stringformat("%0+b", Hex::dumper("abc\x01\x02", 5)) );
        CHECK_THROWS( stringformat("%0 b", Hex::dumper("abc\x01\x02", 5)) );
        CHECK( stringformat("% 0b", Hex::dumper("abc\x01\x02", 5)) == "6162630102  abc.." );
        CHECK_THROWS( stringformat("% +b", Hex::dumper("abc\x01\x02", 5)) );
        CHECK_THROWS( stringformat("% -b", Hex::dumper("abc\x01\x02", 5)) );
        CHECK( stringformat("%+0b", Hex::dumper("abc\x01\x02", 5)) == "abc.." );
        CHECK_THROWS( stringformat("%+-b", Hex::dumper("abc\x01\x02", 5)) );
        CHECK_THROWS( stringformat("%+ b", Hex::dumper("abc\x01\x02", 5)) );
        CHECK( stringformat("%-0b", Hex::dumper("abc\x01\x02", 5)) == "6162630102" );
        CHECK( stringformat("%-+b", Hex::dumper("abc\x01\x02", 5)) == "" );
        CHECK( stringformat("%- b", Hex::dumper("abc\x01\x02", 5)) == "61 62 63 01 02" );

        CHECK( stringformat("%- b", Hex::dumper("", 0)) == "" );
        CHECK( stringformat("%+0b", Hex::dumper("", 0)) == "" );
        CHECK( stringformat("% b", Hex::dumper("", 0)) == "" );
    }
    SECTION("inttypes") {
        CHECK( stringformat("%I64d", 1) == "1" );
        CHECK( stringformat("%lld", 1) == "1" );
        CHECK( stringformat("%hd", 1) == "1" );
        CHECK( stringformat("%ld", 1) == "1" );
        CHECK( stringformat("%d", 1) == "1" );
        CHECK( stringformat("%d", 1L) == "1" );
        CHECK( stringformat("%d", 1U) == "1" );
        CHECK( stringformat("%d", '\x01') == "1" );
        CHECK( stringformat("%d", char(1)) == "1" );
        CHECK( stringformat("%d", uint8_t(1)) == "1" );
        CHECK( stringformat("%d", int8_t(1)) == "1" );
        CHECK( stringformat("%d", short(1)) == "1" );
        CHECK( stringformat("%d", uint16_t(1)) == "1" );
        CHECK( stringformat("%d", int16_t(1)) == "1" );

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

        CHECK( stringformat("%+3d", 1) == "+ 1" );
        CHECK( stringformat("%+3d", 10) == "+10" );
        CHECK( stringformat("%+3d", 0) == "+ 0" );
        CHECK( stringformat("%+3d", -1) == "- 1" );
        CHECK( stringformat("%+3d", -10) == "-10" );

        CHECK( stringformat("%+03d", -1) == "-01" );
        CHECK( stringformat("%+03d",  1) == "+01" );


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
        CHECK( stringformat("%s", std::vector<int>{-1,0,1,2,3,4}) == "-1 0 1 2 3 4" );
        CHECK( stringformat("%,s", std::vector<int>{1,2,3,4}) == "1,2,3,4" );

        CHECK( stringformat("%s", std::vector<double>{1,2,3,4}) == "1 2 3 4" );
        CHECK( stringformat("%s", std::vector<uint8_t>{1,2,3,4}) == "1 2 3 4" );

        CHECK( stringformat("%s", std::vector<char>{-0x80,-1,0,1,2,3,4,0x7f}) == "-128 -1 0 1 2 3 4 127" );
        CHECK( stringformat("%s", std::vector<short>{-0x8000,-1,0,1,2,3,4, 0x7fff}) == "-32768 -1 0 1 2 3 4 32767" );
        CHECK( stringformat("%s", std::vector<long>{-1,0,1,2,3,4}) == "-1 0 1 2 3 4" );
        CHECK( stringformat("%,s", std::vector<std::string>{"abc", "def", "xyz"}) == "abc,def,xyz" );
    }
    SECTION("unprintable") {
        CHECK( stringformat("%s", Unprintable{}) == "<?>" );
#if 0
        // todo - make formatter support this:
        CHECK( stringformat("%s", std::vector<Unprintable>{Unprintable{}}) == "<?>" );
        CHECK( stringformat("%s", std::vector<Unprintable>{Unprintable{},Unprintable{}}) == "<?> <?>" );
        CHECK( stringformat("%,s", std::vector<Unprintable>{Unprintable{},Unprintable{}}) == "<?>,<?>" );
#endif
    }
    SECTION("arrays") {
        CHECK( stringformat("%s", std::array<int, 4>{{1,2,3,4}}) == "1 2 3 4" );
        CHECK( stringformat("%,s", std::array<int, 4>{{1,2,3,4}}) == "1,2,3,4" );

        CHECK( stringformat("%s", std::array<double, 4>{{1,2,3,4}}) == "1 2 3 4" );
        CHECK( stringformat("%s", std::array<uint8_t, 4>{{1,2,3,4}}) == "1 2 3 4" );
    }
    SECTION("custom") {
        CHECK( stringformat("%s", mytype()) == "MYTYPE" );
        CHECK( stringformat("%s", std::vector{mytype{}}) == "MYTYPE" );
        CHECK( stringformat("%s", std::vector{mytype{}, mytype{}}) == "MYTYPE MYTYPE" );
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

#ifdef __clang__
#define NULLREPRESENTATION "0x0"
#else
// gcc
#define NULLREPRESENTATION "0"
#endif

        CHECK( stringformat("%p", (void*)0xABCDEF) == "0xabcdef" );
        CHECK( stringformat("%p", (void*)NULL) == NULLREPRESENTATION );
        CHECK( stringformat("%p", (void*)0xABCDEFABCDEF) == "0xabcdefabcdef" );
        CHECK( stringformat("%p", (void*)NULL) == NULLREPRESENTATION );
        CHECK( stringformat("%p", (void*)0xABCDEF) == "0xabcdef" );
        CHECK( stringformat("%p", (void*)NULL) == NULLREPRESENTATION );
        CHECK( stringformat("%p", (char*)0xABCDEF) == "0xabcdef" );
        CHECK( stringformat("%p", (const char*)0xABCDEF) == "0xabcdef" );
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

