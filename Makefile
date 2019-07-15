# make parameters:
#    D=1        -- disable optimization, for easy debugging
#    COVERAGE=1 -- generate instrumented code
#    DOCTEST=1  -- use doctest instead of catch testing framework
#
# make targets:
#    all  ( default )  - build unittests
#    clean             - delete objects and executables
#    coverage          - run coverage test
#
CXXFLAGS=-g $(if $(D),-O0,-O3) -std=c++1z -Wall
CXXFLAGS+=-I .
LDFLAGS=-g
CDEFS?=$(if $(DOCTEST),-DWITH_DOCTEST,-DWITH_CATCH)
all: unittests

CXXFLAGS+=$(if $(COVERAGE),-fprofile-instr-generate -fcoverage-mapping)
LDFLAGS+=$(if $(COVERAGE),-fprofile-instr-generate -fcoverage-mapping)

CXXFLAGS+=-DSUPPORT_POINTERS

unittests: unittests.o unittests2.o test-asn1.o
	$(CXX) $(LDFLAGS) -o $@  $^

%.o: tests/%.cpp
	$(CXX) $(CXXFLAGS) $(CDEFS) -c $^ -o $@


clean:
	$(RM) unittests $(wildcard *.o) $(wildcard *.profdata *.profraw)

# list of files checked for coverage
COVERAGEFILES=argparse.h arrayview.h asn1parser.h datapacking.h fhandle.h formatter.h fslibrary.h hexdumper.h mmem.h stringconvert.h stringlibrary.h utfconvertor.h

COVOPTIONS+=-show-instantiation-summary
#COVOPTIONS+=-show-functions
COVOPTIONS+=-show-expansions
COVOPTIONS+=-show-regions
COVOPTIONS+=-show-line-counts-or-regions


XCODETOOLCHAINS=$(shell xcode-select -p)/Toolchains/XcodeDefault.xctoolchain
coverage: COVERAGE=1
coverage: unittests
	./unittests
	$(XCODETOOLCHAINS)/usr/bin/llvm-profdata merge -o unittest.profdata default.profraw
	$(XCODETOOLCHAINS)/usr/bin/llvm-cov show ./unittests -instr-profile=unittest.profdata $(COVOPTIONS) $(COVERAGEFILES)

