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
