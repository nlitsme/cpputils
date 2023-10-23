#include <kiss/unittestframework.h>

// include twice to detect proper header behaviour
#include <cpputils/timepoint.h>
#include <cpputils/timepoint.h>

TEST_SUITE("time") {
    TEST_CASE("delta") {
        CHECK( timedelta{1234567}.totalusec() == 1234567 );
        CHECK( timedelta{1234567}.usec() == 234567 );
        CHECK( timedelta{1234567}.msec() == 234 );
        CHECK( timedelta{61234567}.seconds() == 1 );
        CHECK( timedelta{1234567}.totalmsec() == 1234 );
        CHECK( timedelta{1234567}.totalseconds() == 1 );
    }
    TEST_CASE("point") {
        CHECK( timepoint{} == timepoint{} );
        CHECK( timepoint{}.empty() );
        CHECK( !timepoint::now().empty() );
        CHECK( !timepoint{123131299313213}.empty() );
        CHECK( timepoint{123456789123456} == timepoint{123456789123456} );
        CHECK( timepoint{123456789123456}.time() == 123456789 );
        CHECK( timepoint{123456789123456}.usec() == 123456 );
        CHECK( (double)timepoint{123456789123456} == 123456789.123456 );
        CHECK( (int64_t)timepoint{123456789123456} == 123456789123456 );
    }
    TEST_CASE("arithmetic") {
        CHECK( timepoint{123456789123456} + timedelta{0} == timepoint{123456789123456} );
        CHECK( timepoint{123456789123456} - timedelta{0} == timepoint{123456789123456} );
        CHECK( timedelta{0} + timepoint{123456789123456} == timepoint{123456789123456} );

        CHECK( timepoint{123456789123456} + timedelta{321} == timepoint{123456789123777} );
        CHECK( timepoint{123456789123456} - timedelta{123} == timepoint{123456789123333} );
        CHECK( timedelta{321} + timepoint{123456789123456} == timepoint{123456789123777} );

        CHECK( timepoint{123456789123456} - timepoint{123456789000000} == timedelta{123456} );
        CHECK( timedelta{123456} - timedelta{123} == timedelta{123333} );
        CHECK( timedelta{123456} + timedelta{321} == timedelta{123777} );
    }
    TEST_CASE("streams") {
        CHECK( stringformat("%s", timepoint{123456789123456}) == "123456789.123456" );
        CHECK( stringformat("%s", timepoint{123456788999999}) == "123456788.999999" );
        CHECK( stringformat("%s", timepoint{123456789000000}) == "123456789.000000" );
        CHECK( stringformat("%s", timepoint{123456789000001}) == "123456789.000001" );
        CHECK( stringformat("%s", timedelta{123}) == "0.000123" );
    }
}

