#include "unittestframework.h"
#include <vector>

// include twice to detect proper header behaviour
#include <cpputils/gsmkcrc.h>
#include <cpputils/gsmkcrc.h>

TEST_SUITE("crc16") {

    TEST_CASE("crc24") {
        std::vector<uint8_t> v22(33, 0x22);
        std::vector<uint8_t> v33(22, 0x33);
        CHECK( gsmkcrc24(v22.data(), v22.size()) == 0xffd4eb  );       // crc-24
        CHECK( gsmkcrc32(v22.data(), v22.size()) == 0x38256711  );     // crc32-bzip2

        CHECK( gsmkcrc24(v33.data(), v33.size()) == 0x8e9eb6  );
        CHECK( gsmkcrc32(v33.data(), v33.size()) == 0x60263fbe  );
    }
}


