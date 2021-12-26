CMAKEARGS+=$(if $(D),-DCMAKE_BUILD_TYPE=Debug,-DCMAKE_BUILD_TYPE=Release)
CMAKEARGS+=$(if $(COV),-DOPT_COV=1)
CMAKEARGS+=$(if $(PROF),-DOPT_PROF=1)
CMAKEARGS+=$(if $(LIBCXX),-DOPT_LIBCXX=1)
CMAKEARGS+=$(if $(STLDEBUG),-DOPT_STL_DEBUGGING=1)

cmake:
	cmake -B build . $(CMAKEARGS)
	$(MAKE) -C build $(if $(V),VERBOSE=1)

vc:
	"C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe" -G"Visual Studio 16 2019" -B build . $(CMAKEARGS)
	"C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/MSBuild/Current/Bin/amd64/MSBuild.exe" build/*.sln -t:Rebuild

llvm:
	CC=clang CXX=clang++ cmake -B build . $(CMAKEARGS)
	$(MAKE) -C build $(if $(V),VERBOSE=1)


clean:
	$(RM) -r build CMakeFiles CMakeCache.txt CMakeOutput.log
	$(RM) $(wildcard *.gcov)

COVOPTIONS+=-show-instantiation-summary
#COVOPTIONS+=-show-functions
COVOPTIONS+=-show-expansions
COVOPTIONS+=-show-regions
COVOPTIONS+=-show-line-counts-or-regions

COVERAGEFILES=HiresTimer.h argparse.h arrayview.h asn1parser.h b32-alphabet.h b64-alphabet.h base32encoder.h base64encoder.h crccalc.h
COVERAGEFILES+=datapacking.h fhandle.h formatter.h fslibrary.h hexdumper.h is_stream_insertable.h mmem.h mmfile.h xmlnodetree.h xmlparser.h
COVERAGEFILES+=string-base.h string-join.h string-lineenum.h string-parse.h string-split.h string-strip.h stringconvert.h stringlibrary.h templateutils.h utfconvertor.h utfcvutils.h

coverage:
	llvm-profdata merge -o unittest.profdata default.profraw
	llvm-cov show ./build/unittests -instr-profile=unittest.profdata $(COVOPTIONS) $(COVERAGEFILES)

gcov:
	gcov -H $(wildcard build/CMakeFiles/unittests.dir/tests/*.cpp.gcda)
