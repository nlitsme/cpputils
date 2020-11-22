#define UNITTESTMAIN
#include "unittestframework.h"

// including all here again, so we will catch linking errors.
#include "argparse.h"
#include "asn1parser.h"
#include "base64encoder.h"
#include "datapacking.h"
#include "fhandle.h"
#include "formatter.h"
#include "hexdumper.h"
#include "string-strip.h"
#include "stringconvert.h"
#include "stringlibrary.h"

TEST_CASE("main") {
    CHECK(true);
}
