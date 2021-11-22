#include "unittestframework.h"

#include <signal.h>

#include "mmem.h"
#include "mmem.h"
#include "mmfile.h"
#include "mmfile.h"
#include "formatter.h"

#include <vector>
#include <cstdlib>

TEST_CASE("mmem0") {

    // create a virtual file
    filehandle f(memfd_create("test.dat", 0));

    // check that we can create a mapping from an empty file.

    mappedmem m(f, 0, 0x1000000);

    CHECK(true);
}


TEST_CASE("mmem") {

    std::vector<uint8_t> rnddata(256);
    for (auto &b : rnddata)
        b = std::rand();

    // create a virtual file
    filehandle f(memfd_create("test.dat", 0));
    f.trunc(0x1234);

    mappedmem m(f, 0, 0x1000000);

    auto p0 = m.begin();

    // init with some random data
    std::copy(rnddata.begin(), rnddata.end(), p0);

    // resize
    f.trunc(0x12340);
    m.resize(0x12340);

    auto p1 = m.begin();

    // check that ptr and contents stayed the same
    CHECK( p0 == p1 );
    CHECK( std::equal(rnddata.begin(), rnddata.end(), p0) );

    // resize again
    f.trunc(0x1234);
    m.resize(0x1234);

    auto p2 = m.begin();

    // check that ptr and contents stayed the same
    CHECK( p0 == p2 );
    CHECK( std::equal(rnddata.begin(), rnddata.end(), p0) );


}


TEST_CASE("mmem2") {

    std::vector<uint8_t> rnddata(512);
    for (auto &b : rnddata)
        b = std::rand();

    // create a virtual file
    filehandle f1(memfd_create("test.dat", 0));
    f1.trunc(0x1234);

    mappedmem m1(f1, 0, 0x1000000000);
    auto p0 = m1.begin();

    // init with some random data
    std::copy(rnddata.begin(), rnddata.begin()+256, p0);


    // create a virtual file
    filehandle f2(memfd_create("test2.dat", 0));
    f2.trunc(0x1234);

    mappedmem m2(f2, 0, 0x1000000);
    auto q0 = m2.begin();

    // init with some random data
    std::copy(rnddata.begin()+256, rnddata.end(), q0);

    // resize
    f1.trunc(0x10000000);
    m1.resize(0x10000000);

    auto p1 = m1.begin();

    // check that ptr and contents stayed the same
    CHECK( p0 == p1 );
    CHECK( std::equal(rnddata.begin(), rnddata.begin()+256, p0) );

    // resize again
    f1.trunc(0x100000000);
    m1.resize(0x100000000);

    auto p2 = m1.begin();

    // check that ptr and contents stayed the same
    CHECK( p0 == p2 );
    CHECK( std::equal(rnddata.begin(), rnddata.begin()+256, p0) );
}
