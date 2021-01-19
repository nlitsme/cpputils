#include "unittestframework.h"
#include "formatter.h"

#include "arrayview.h"
#include "arrayview.h"

TEST_CASE("av") {
    CHECK( stringformat("%b", array_view<const uint8_t*>((const uint8_t*)"te\n\tst", 6)) == "74 65 0a 09 73 74" );
    CHECK( stringformat("%+b", array_view<const uint8_t*>((const uint8_t*)"te\n\tst", 6)) == "74 65 0a 09 73 74" );
    CHECK( stringformat("%-b", array_view<const uint8_t*>((const uint8_t*)"te\n\tst", 6)) == "74 65 0a 09 73 74" );

    CHECK( stringformat("%b", array_view<const char*>("te\n\tst", 6)) == "74 65 0a 09 73 74" );
    CHECK( stringformat("%-b", array_view<const char*>("te\n\tst", 6)) == "74 65 0a 09 73 74" );
}
