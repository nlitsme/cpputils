#include "unittestframework.h"
#include <vector>

// include twice to detect proper header behaviour
#include <cpputils/crccalc.h>
#include <cpputils/crccalc.h>
#include <cstdint>

template<typename P>
uint32_t crc24_1(P ptr, size_t size)
{
    static CrcCalc<uint32_t, 0x1864cfb, 24> CRC;
    return CRC.add(0xb704ce, ptr, size);
}
template<typename P>
uint32_t crc24_2(P ptr, size_t size)
{
    static CrcCalc<uint32_t, 0x864cfb, 24> CRC;
    return CRC.add(0xb704ce, ptr, size);
}

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
    TEST_CASE("crc24") {
        std::vector<uint8_t> v22(33, 0x22);
        std::vector<uint8_t> v33(22, 0x33);
        CHECK( crc16(v22.data(), v22.size()) == 0x73b9 );              // x-25
        CHECK( crc32(v22.data(), v22.size()) == 0xee89f6e5  );         // crc-32
        CHECK( crc24_1(v22.data(), v22.size()) == 0x04E4F7B );
        CHECK( crc24_2(v22.data(), v22.size()) == 0x0F95BF2 );

        CHECK( crc16(v33.data(), v33.size()) == 0x9ba7  );
        CHECK( crc32(v33.data(), v33.size()) == 0x70e447b9  );
        CHECK( crc24_1(v33.data(), v33.size()) == 0x17F224B );
        CHECK( crc24_2(v33.data(), v33.size()) == 0x0A38C2A );
    }
}

