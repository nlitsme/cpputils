#include "unittestframework.h"
#include <cpputils/fslibrary.h>
#include <cpputils/fslibrary.h>
#include <cpputils/formatter.h>

TEST_CASE("fileenum") {
    int n = 0;
    for (auto [p, e] : fileenumerator("."))
    {
        //print("p:%s\n\te:%s\n", p, e);
        n++;
    }
    CHECK(n>0);
}
