CMAKEARGS+=$(if $(D),-DCMAKE_BUILD_TYPE=Debug,-DCMAKE_BUILD_TYPE=Release)
CMAKEARGS+=$(if $(COV),-DOPT_COV=1)
CMAKEARGS+=$(if $(PROF),-DOPT_PROF=1)
CMAKEARGS+=$(if $(LIBCXX),-DOPT_LIBCXX=1)
CMAKEARGS+=$(if $(STLDEBUG),-DOPT_STL_DEBUGGING=1)
CMAKEARGS+=$(if $(SANITIZE),-DOPT_SANITIZE=1)
CMAKEARGS+=$(if $(ANALYZE),-DOPT_ANALYZE=1)
CMAKEARGS+=$(if $(BENCH),-DOPT_BENCH=1)

cmake:
	cmake -B build . $(CMAKEARGS)
	cmake --build build $(if $(V),--verbose)

ctest: TEST=1
ctest: cmake
	cd build && ctest --verbose

VC_CMAKE=C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe
vc:
	"$(VC_CMAKE)" -G"Visual Studio 16 2019" -B build . $(CMAKEARGS)
	"$(VC_CMAKE)" --build build $(if $(V),--verbose)

llvm:
	CC=clang CXX=clang++ cmake -B build . $(CMAKEARGS)
	cmake --build build $(if $(V),--verbose)

SCANBUILD=$(firstword $(wildcard /usr/bin/scan-build*))
llvmscan:
	CC=clang CXX=clang++ cmake -B build . $(CMAKEARGS)
	$(SCANBUILD) cmake --build build $(if $(V),--verbose)


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

coverage:  ctest
	llvm-profdata merge -o unittest.profdata default.profraw
	llvm-cov show ./build/unittests -instr-profile=unittest.profdata $(COVOPTIONS) $(COVERAGEFILES)

gcov:  ctest
	find build/CMakeFiles -name "*.gcda" | xargs rm -f
	./build/unittests
	rm -f *.gcov
	#  -H : human readable output
	#  -p : preserve paths
	#  -m : demangle names
	find build/CMakeFiles -name "*.gcda" | xargs gcov -p -m -H
	# outputs #home#itsme@workprj...  files in the current directory
	#  look for lines with '#####', this means: never executed.
	rm -f *"#build#"* *"#tests#"*  "#usr#"*
	head -999999 *.gcov | filtercov 

