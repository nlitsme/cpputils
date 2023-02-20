#define UNITTESTMAIN
#include "unittestframework.h"

// including all here again, so we will catch linking errors.
#include <cpputils/argparse.h>
#include <cpputils/asn1parser.h>
#include <cpputils/base64encoder.h>
#include <cpputils/datapacking.h>
#include <cpputils/fhandle.h>
#include <cpputils/formatter.h>
#include <cpputils/hexdumper.h>
#include <cpputils/string-strip.h>
#include <cpputils/stringconvert.h>
#include <cpputils/stringlibrary.h>

TEST_CASE("main") {
    CHECK(true);
}
