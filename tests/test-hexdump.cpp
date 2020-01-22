#include "unittestframework.h"


#include "hexdumper.h"

#include <sstream>
#include <vector>
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
        buf << Hex::singleline << std::showpos <<  Hex::dumper("abcde", 5);
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
    SECTION("case") {
        std::stringstream buf;
        SECTION("lower") {
            buf << Hex::hexstring << std::nouppercase << Hex::dumper("jklmn", 5);
            CHECK( buf.str() == "6a6b6c6d6e" );
        }
        SECTION("upper") {
            buf << Hex::hexstring << std::uppercase << Hex::dumper("jklmn", 5);
            CHECK( buf.str() == "6A6B6C6D6E" );
        }
    }
    SECTION("numberbase") {
        std::stringstream buf;
        SECTION("hex") {
            buf << Hex::singleline << std::left << std::hex << Hex::dumper("abcde", 5);
            CHECK( buf.str() == "61 62 63 64 65" );
        }
        SECTION("dec") {
            buf << Hex::singleline << std::left << std::dec << Hex::dumper("abcde", 5);
            CHECK( buf.str() == "97 98 99 100 101" );
        }
        SECTION("oct") {
            buf << Hex::singleline << std::left << std::oct << Hex::dumper("abcde", 5);
            CHECK( buf.str() == "141 142 143 144 145" );
        }
        SECTION("bin") {
            buf << Hex::singleline << std::left << Hex::bin << Hex::dumper("abcde", 5);
            CHECK( buf.str() == "01100001 01100010 01100011 01100100 01100101" );
        }
        SECTION("hex.showbase") {
            buf << Hex::singleline << std::left << std::hex << std::showbase << Hex::dumper("abcde", 5);
            CHECK( buf.str() == "0x61 0x62 0x63 0x64 0x65" );
        }
        SECTION("dec.showbase") {
            buf << Hex::singleline << std::left << std::dec << std::showbase << Hex::dumper("abcde", 5);
            CHECK( buf.str() == "97 98 99 100 101" );
        }
        SECTION("oct.showbase") {
            buf << Hex::singleline << std::left << std::oct << std::showbase << Hex::dumper("abcde", 5);
            CHECK( buf.str() == "0141 0142 0143 0144 0145" );
        }
        SECTION("bin.showbase") {
            buf << Hex::singleline << std::left << Hex::bin << std::showbase << Hex::dumper("abcde", 5);
            CHECK( buf.str() == "0b01100001 0b01100010 0b01100011 0b01100100 0b01100101" );
        }

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
"67 68 69 6a 6b 6c 6d 6e 6f 70 71                 ghijklmnopq     \n");
    }
    SECTION("offset") {
        std::stringstream buf;
        buf << Hex::offset(0xa000) << Hex::multiline <<  Hex::dumper("0123456789abcdefghijklmnopq", 27);
        CHECK( buf.str() == 
"0000a000: 30 31 32 33 34 35 36 37 38 39 61 62 63 64 65 66  0123456789abcdef\n"
"0000a010: 67 68 69 6a 6b 6c 6d 6e 6f 70 71                 ghijklmnopq     \n");
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
    SECTION("summarize") {
        // this test currently fails
        std::stringstream buf;
        SECTION("plain summary") {
            buf << Hex::offset(0) << std::setw(2) <<  Hex::dumper("012323232323cdefghghghghopq", 27);
            CHECK( buf.str() == 
    "00000000: 30 31  01\n"
    "00000002: 32 33  23\n"
    "* [ 0x4 lines ]\n"
    "0000000c: 63 64  cd\n"
    "0000000e: 65 66  ef\n"
    "00000010: 67 68  gh\n"
    "* [ 0x3 lines ]\n"
    "00000018: 6f 70  op\n"
    "0000001a: 71     q \n"
    );
        }
        SECTION("summary,th=3") {
            buf << Hex::offset(0) << std::setw(2) << Hex::summarize_threshold(3) <<  Hex::dumper("012323232323cdefghghghghopq", 27);
            CHECK( buf.str() == 
    "00000000: 30 31  01\n"
    "00000002: 32 33  23\n"
    "* [ 0x4 lines ]\n"
    "0000000c: 63 64  cd\n"
    "0000000e: 65 66  ef\n"
    "00000010: 67 68  gh\n"
    "00000012: 67 68  gh\n"
    "00000014: 67 68  gh\n"
    "00000016: 67 68  gh\n"
    "00000018: 6f 70  op\n"
    "0000001a: 71     q \n"
    );
        }
        SECTION("noskip") {
            buf << Hex::offset(0) << std::setw(2) << std::noskipws <<  Hex::dumper("012323232323cdefghghghghopq", 27);
            CHECK( buf.str() == 
    "00000000: 30 31  01\n"
    "00000002: 32 33  23\n"
    "00000004: 32 33  23\n"
    "00000006: 32 33  23\n"
    "00000008: 32 33  23\n"
    "0000000a: 32 33  23\n"
    "0000000c: 63 64  cd\n"
    "0000000e: 65 66  ef\n"
    "00000010: 67 68  gh\n"
    "00000012: 67 68  gh\n"
    "00000014: 67 68  gh\n"
    "00000016: 67 68  gh\n"
    "00000018: 6f 70  op\n"
    "0000001a: 71     q \n"
    );
        }

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

