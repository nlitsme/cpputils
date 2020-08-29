#include "unittestframework.h"
#include <vector>

// include twice to detect proper header behaviour
#include "crccalc.h"
#include "crccalc.h"

TEST_SUITE("crc16") {
    TEST_CASE("checkcrc") {
        std::vector<uint8_t> pkt0{0x00};
        std::vector<uint8_t> pkt1{0x13, 0x00, 0x7e};

        CHECK( crc16(pkt0) == 0xf078 );
        CHECK( crc16(pkt1) == 0x36c4 );

        CHECK( crc32(pkt0) == 0xd202ef8d );
        CHECK( crc32(pkt1) == 0x569c9800 );
    }
    TEST_CASE("crccalc") {
        CrcCalc<uint16_t, 0x8408, 16> mycrc16;
        CrcCalc<uint32_t, 0xEDB88320, 32> mycrc32;

        std::vector<uint8_t> pkt0{0x00};
        std::vector<uint8_t> pkt1{0x13, 0x00, 0x7e};

        CHECK( mycrc16.calc(&pkt0[0], pkt0.size()) == 0xf078 );
        CHECK( mycrc16.calc(&pkt1[0], pkt1.size()) == 0x36c4 );

        CHECK( mycrc32.calc(&pkt0[0], pkt0.size()) == 0xd202ef8d );
        CHECK( mycrc32.calc(&pkt1[0], pkt1.size()) == 0x569c9800 );

    }
}


