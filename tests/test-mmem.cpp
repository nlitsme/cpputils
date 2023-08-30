#include "unittestframework.h"

#include <signal.h>

#include <cpputils/mmem.h>
#include <cpputils/mmem.h>
#include <cpputils/mmfile.h>
#include <cpputils/mmfile.h>
#include <cpputils/formatter.h>

#include <vector>
#include <cstdlib>

#if defined(__MACH__) || defined(_WIN32)
// simulate the linux memfd_create function
int memfd_create(const char *name, int flags)
{
    char uname[L_tmpnam];
    if (std::tmpnam(uname)) {
        int f = open(uname, O_CREAT|O_RDWR, 0666);
        std::remove(uname);
        return f;
    }
    return -1;
}
#endif

TEST_CASE("mmem0") {

    // create a virtual file
#ifdef _WIN32
    int f = -1;
#else
    filehandle f(memfd_create("test.dat", 0));
#endif

    // check that we can create a mapping from an empty file.

    mappedmem m(f, 0, 0x1000000);

    CHECK(m.size() == 0x1000000);
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
#ifndef _WIN32
    // note: on win32 you can't trunc a file which has a mapping.
    f.trunc(0x12340);
#ifdef MREMAP_MAYMOVE
    m.resize(0x12340);
#endif
#endif

    auto p1 = m.begin();

    // check that ptr and contents stayed the same
    CHECK( p0 == p1 );
    CHECK( std::equal(rnddata.begin(), rnddata.end(), p0) );

#ifndef _WIN32
    // resize again
    f.trunc(0x1234);
#ifdef MREMAP_MAYMOVE
    m.resize(0x1234);
#endif
#endif

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
#ifndef _WIN32
    f1.trunc(0x10000000);
#ifdef MREMAP_MAYMOVE
    m1.resize(0x10000000);
#endif
#endif
    auto p1 = m1.begin();

    // check that ptr and contents stayed the same
    CHECK( p0 == p1 );
    CHECK( std::equal(rnddata.begin(), rnddata.begin()+256, p0) );

    // resize again
#ifndef _WIN32
    f1.trunc(0x100000000);
#ifdef MREMAP_MAYMOVE
    m1.resize(0x100000000);
#endif
#endif

    auto p2 = m1.begin();

    // check that ptr and contents stayed the same
    CHECK( p0 == p2 );
    CHECK( std::equal(rnddata.begin(), rnddata.begin()+256, p0) );
}

