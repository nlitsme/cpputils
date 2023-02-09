#include "unittestframework.h"

#include <cpputils/base32encoder.h>
#include <cpputils/base32encoder.h>

#include <array>

TEST_CASE("base32") {
    enum { E=1, D=2, FD=4 };
    std::vector<std::tuple<std::vector<uint8_t>, int, std::string>> testcases = {
        { { }, E|D, "" },
        { { }, D,   "\n" },
        { { }, D,   "\t" },
        { { }, FD,   "." },
        { { }, D,   "\t" },
        { { }, D,   "=" },
        { { }, D,   "==" },
        { { }, D,   "===" },

        // from rfc4648
        { { 0x66                          }, E|D, "MY======" },
        { { 0x66,0x6f                     }, E|D, "MZXQ====" },
        { { 0x66,0x6f,0x6f                }, E|D, "MZXW6===" },
        { { 0x66,0x6f,0x6f,0x62           }, E|D, "MZXW6YQ=" },
        { { 0x66,0x6f,0x6f,0x62,0x61      }, E|D, "MZXW6YTB" },
        { { 0x66,0x6f,0x6f,0x62,0x61,0x72 }, E|D, "MZXW6YTBOI======" },


    };
    SECTION("cases") {
        for (auto& [data, flags, txt] : testcases) {
            if (flags&E) {
                auto e = base32_encode(data);
                CHECK(e == txt);
            }
            if (flags&D) {
                auto d = base32_decode(txt);
                CHECK(d == data);
            }
            if (flags&FD) {
                CHECK_THROWS(base32_decode(txt));
            }
        }
    }
    SECTION("types") {
        std::array<uint8_t, 3> a = { 0, 1, 2 };
        CHECK( base32_encode(a) == "AAAQE===" );

        std::vector<uint8_t> v = { 0, 1, 2 };
        CHECK( base32_encode(v) == "AAAQE===" );

        std::basic_string<uint8_t> bs = { 0, 1, 2 };
        CHECK( base32_encode(bs) == "AAAQE===" );
        CHECK( base32_encode((std::basic_string_view<uint8_t>)bs ) == "AAAQE===" );

        std::vector<int> iv = { 0, 1, 2 };
        CHECK( base32_encode(iv) == "AAAQE===" );

        std::string txt = "AAAQE===";
        CHECK( base32_decode(txt) == std::vector<uint8_t>{0, 1, 2} );
        CHECK( base32_decode((std::string_view)txt) == std::vector<uint8_t>{0, 1, 2} );

        std::basic_string<int> itxt = { 'A', 'A', 'A', 'Q', 'E', '=', '=', '=' };

        CHECK( base32_decode(itxt) == std::vector<uint8_t>{0, 1, 2} );
        CHECK( base32_decode((std::basic_string_view<int>)itxt) == std::vector<uint8_t>{0, 1, 2} );

        std::basic_string<uint8_t> btxt = { 'A', 'A', 'A', 'Q', 'E', '=', '=', '=' };
        CHECK( base32_decode(btxt) == std::vector<uint8_t>{0, 1, 2} );
        CHECK( base32_decode((std::basic_string_view<uint8_t>)btxt) == std::vector<uint8_t>{0, 1, 2} );
    }
    SECTION("invalid") {
        std::string txt = "x";

        for (int i = 0 ; i < 256 ; i++)
        {
            if (i == '\t' || i == '\n' || i == '\r' || i == 0xc || i == ' '
                    || (i>='2' && i<='7')
                    || (i>='a' && i<='z')
                    || (i>='A' && i<='Z')
                    || i=='=')
                continue;
            txt[0] = i;

            INFO("checking char " << i);
            CHECK_THROWS( base32_decode(txt) );
        }
    }
}


