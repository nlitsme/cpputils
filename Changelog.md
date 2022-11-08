## Tue Nov  8 15:58:20 CET 2022
 * improved `packer_base` for `back_inserter`.
 * fhandle: fixed bug with return value of read/write of non-byte sized types
 * added ctest support
 * added cmake finders for doctest and fmt libraries.

## Fri May 27 22:45:46 CEST 2022
 * updated doctest from 2.3.6 to 2.4.8
 * updated catch from 2.11.1 to v2.13.9
 * added `OPT_ANALYZE` and `OPT_SANITIZE` for the gcc/clang builtin analyzers
 * fixed several stl-debug warnings. more exceptions in asn1parser
 * added option to enable STLDEBUGGING, targets for coverage processing
 * all my projects now use `USE_xxx`
 * base64encoder: added `base64_encode_unpadded`
 * solved problem with `operator<<(std::something)` output on some compilers.  renamed 'debug' to 'windebug' to avoid name collisions.
 * added cmakeargs, same code in all makefiles for building msvc
 * added boost as depenency for the `fmt*boost.cpp` benchmarks
 * tests for mmem.h
 * makefile now only contains code for invoking cmake, the old makefile was renamed to Makefile.linux
 * fixed some incompatibilities with gcc and clang. moved `is_stream_insertable` to separate file.
 * base64: more tests
 * xmlparser and xmlnodetree
 * jenkins: added freebsd, stdlib variants
 * changed fmt bench to print and string variants
 * make/cmake: added COV, PROF, LIBCXX flags. llvm target
 * asn1parser: added gettlv(p,p) function
 * default to the cmake build
 * test-strlib.cpp: added hex2binary tests
 * test-argparse.cpp: added check for single arg
 * hex2binary now works on arrays as well
 * added 'data' method to `array_view`
 * change old makefile to c++20
 * cmake: added formatter benchmarks. for bench: fetch fmt library
 * several bugs fixed
 * more tests

## Thu Apr  8 09:59:13 CEST 2021
 * mmfile: better handling of ro/rw flags
 * added cmake build
 * more tests
 * alphabets for base32 and base64 coding
 * fhandle: write now takes PTR+size. added pread, pwrite.
 * formatter: added hex,octal output for 128-bit integers
 * added `string_to_signed` integer parsers.
 * mmfile no longer throws on an empty file.

bugfixes:
 * formatter: reset to decimal in '%s'
 * mmem: forgot to copy 'length' field in the move constructor.
 * correct hex output of `array_view`

## Mon 14 Sep 2020 07:19:25 PM CEST
 * added a base32 encoder/decoder
 * fixed lineenum, now all tests succeed.

## Sat Aug 29 22:20:06 CEST 2020
 * string-lineenum.h: enumerate lines in a string
 * templateutils: is\_container, is\_stream\_insertable, is\_callable, is\_searchable.
 * formatter.h: fixed bug in %b handling
 * datapacking: added setbytes with first+last pair.
 * datapacking: added 24-bit support. added z-string support.
 * datapacking: improved container detection for makeXXpacker. setbytes now takes generic container.
 * asn1parser: added named constants for classes and tags
 * mappedfile: added mode flags
 * added filehandle support to 'fprint'
 * fhandle: added 'opening' constructor, which takes a filename and flags.
 * crc32 + crc16 code
 * more tests
 * more documentation.


## Wed Mar  4 12:02:54 CET 2020
 * fhandle: added operator bool
 * added simpler string conversion functions
 * fixed bugs in stringsplit
 * more tests

## Wed Jan 22 22:46:09 CET 2020

 * updated catch from v2.3.0 to v2.11.1
 * updated doctest from v1.2.0 to v2.3.6
 * split tests in one file per header, unittests now takes all files from tests directory
 * utfcvutils - ifdeffed clang specific code
 * mmem: noting problem with mapping block/char devices.
 * formatter: improved printing of vector, array. added printing of std::map, std::set. added int128 support. fixed '%+03d' formatting
 * fhandle: now keeps a sharedptr to the actual filehandle, so filehandles can be copied. added seek, tell, trunc, write, read methods
 * datapacking: added skip. simplified 'is_container' check.
 * unittests for fhandle
 * mmfile: wraps both a filehandle and mmap
 * copied hirestimer from itslib
 * added base64 en + decoding
 * unitests for base64
 * stringconvert: added some explanation
 * string-base: added 'beginswith'
 * fslibrary: added filetype filter. added dirent in return value
 * arrayview: added makerange(p, size)
 * argparse: added 'getfullarg' method, which returns the complete current argument
 * defaulting to doctest for unittest framework
