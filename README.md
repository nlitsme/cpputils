# cpputils
various c++ utility classes

* iostreams based hexdumper utility
* string formatter using variadic templates instead of varargs,
  making use of iostreams for formatting.
* utf-X string converter.


## hexdumper

Using iostream to configure hexdump output:

    std::cout << hex::hexstring << hex::dumper(data, size) << "\n";
    std::cout << hex::offset(0x12000) << hex::right << hex::dumper(data, size) << "\n";
    
A more detailed description can be found [in this blog post](http://nlitsme.github.io/posts/hexdumper-for-c%2B%2B-iostreams/)


## formatter

printf like formatting using a combination of variadic templates and iostreams.

Example:

    std::cout << stringformat("%d %s %d", 1LL, std::string("test"), size_t(3));

You can stringify custom types by defining a suitable `operator<<(os, customtype)`.


## stringconvert

Convert C or C++ strings to C++ strings, between any of UTF-8, UTF-16, UTF-32, depending on the size of the character type.

Example:

    auto utf8str = string::convert<char>(L"test");
    auto utf16str = string::convert<uint16_t>(L"test");

The first line converts a wchar\_t string, which is either utf-16 or utf-32 encoded depending on the compiler,
to a utf-8 string, the second line converts to utf-16.


### todo

 * add support for hexdumping data from streams.
 * string alignment / width does not work correctly for unicode characters > 0x80.
 * `"% d"` : space-for-positive is not supported.
 * `"%*d"` : width from argument list is not supported.
 * `"%+08d"`  produces the wrong result: the sign will be after the padding, instead of in front.
 * `"%.8s"`  string truncation does not work.


(C) 2016 Willem Hengeveld <itsme@xs4all.nl>
