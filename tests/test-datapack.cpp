#include "unittestframework.h"

#include "datapacking.h"
TEST_CASE("packer") {
    SECTION("limits") {
        SECTION("empty") {
            std::vector<uint8_t> data;

            unpacker p(data.begin(), data.end());

            CHECK_THROWS(p.get8());
            CHECK_THROWS(p.get32le());
            CHECK_THROWS(p.getstr(1));
            CHECK_THROWS(p.getbytes(1));
            CHECK(p.getstr(0) == std::string());
            CHECK(p.getbytes(0) == std::vector<uint8_t>());

            packer q(data.begin(), data.end());

            CHECK_THROWS(q.set8(0));
            CHECK_THROWS(q.set32le(0));
            CHECK_THROWS(q.setstr("x"));
            CHECK_THROWS(q.setbytes({0}));
            CHECK_NOTHROW(q.setstr(""));
            CHECK_NOTHROW(q.setbytes({}));

        }
        SECTION("onebyte_8") {
            std::vector<uint8_t> data(1);

            unpacker p(data.begin(), data.end());

            CHECK(p.get8() == 0);
            CHECK_THROWS(p.get8());
        }
        SECTION("onebyte_16") {
            std::vector<uint8_t> data(1);

            unpacker p(data.begin(), data.end());

            CHECK_THROWS(p.get16le());
        }
        SECTION("onebyte_str") {
            std::vector<uint8_t> data(1);

            unpacker p(data.begin(), data.end());

            CHECK(p.getbytes(1) == std::vector<uint8_t>{0x00});
        }
    }

    SECTION("set/getiter") {
        std::vector<uint8_t> data(29);

        packer p(data.begin(), data.end());

        p.set32le(0x11223344);
        p.set16be(0x5566);
        p.set32be(0x11223344);
        p.set16le(0x5566);
        p.set8(0x88);
        p.set64be(0x123456789abcdef0LL);
        p.set64le(0x123456789abcdef0LL);
        CHECK_THROWS(p.set8(0));

        // check consistency
        unpacker q(data.begin(), data.end());

        CHECK(q.get32le() == 0x11223344);
        CHECK(q.get16be() == 0x5566);
        CHECK(q.get32be() == 0x11223344);
        CHECK(q.get16le() == 0x5566);
        CHECK(q.get8() == 0x88);
        CHECK(q.get64be() == 0x123456789abcdef0LL);
        CHECK(q.get64le() == 0x123456789abcdef0LL);
        CHECK_THROWS(q.get8());

        // check be / le
        unpacker r(data.begin(), data.end());

        CHECK(r.get32be() == 0x44332211);
        CHECK(r.get16le() == 0x6655);
        CHECK(r.get32le() == 0x44332211);
        CHECK(r.get16be() == 0x6655);
        CHECK(r.get8() == 0x88);
        CHECK(r.get64le() == 0xf0debc9a78563412LL);
        CHECK(r.get64be() == 0xf0debc9a78563412LL);
        CHECK_THROWS(r.get8());

    }
    SECTION("set/getptr") {
        std::vector<uint8_t> data(29);

        packer p(&data[0], &data[0]+data.size());

        p.set32le(0x11223344);
        p.set16be(0x5566);
        p.set32be(0x11223344);
        p.set16le(0x5566);
        p.set8(0x88);
        p.set64be(0x123456789abcdef0LL);
        p.set64le(0x123456789abcdef0LL);
        CHECK_THROWS(p.set8(0));

        // check consistency
        unpacker q(&data[0], &data[0]+data.size());

        CHECK(q.get32le() == 0x11223344);
        CHECK(q.get16be() == 0x5566);
        CHECK(q.get32be() == 0x11223344);
        CHECK(q.get16le() == 0x5566);
        CHECK(q.get8() == 0x88);
        CHECK(q.get64be() == 0x123456789abcdef0LL);
        CHECK(q.get64le() == 0x123456789abcdef0LL);
        CHECK_THROWS(q.get8());

        // check be / le
        unpacker r(&data[0], &data[0]+data.size());

        CHECK(r.get32be() == 0x44332211);
        CHECK(r.get16le() == 0x6655);
        CHECK(r.get32le() == 0x44332211);
        CHECK(r.get16be() == 0x6655);
        CHECK(r.get8() == 0x88);
        CHECK(r.get64le() == 0xf0debc9a78563412LL);
        CHECK(r.get64be() == 0xf0debc9a78563412LL);
        CHECK_THROWS(r.get8());
    }
    SECTION("set/getstr") {
        std::vector<uint8_t> data(8);

        packer p(&data[0], &data[0]+data.size());

        p.set32le(0x41424344);
        p.setstr("abcd");

        unpacker q(&data[0], &data[0]+data.size());
        CHECK(q.getstr(4) == "DCBA");
        CHECK(q.get32le() == 0x64636261);
    }

    SECTION("backinsert") {
        std::vector<uint8_t> data;

        packer p(std::back_inserter(data), std::back_inserter(data));

        p.set32le(0x11223344);
        p.set16be(0x5566);
        p.set32be(0x11223344);
        p.set16le(0x5566);

        CHECK(data.size() == 12);

        // check consistency
        unpacker q(data.begin(), data.end());

        CHECK(q.get32le() == 0x11223344);
        CHECK(q.get16be() == 0x5566);
        CHECK(q.get32be() == 0x11223344);
        CHECK(q.get16le() == 0x5566);
        CHECK_THROWS(q.get8());
    }
}

