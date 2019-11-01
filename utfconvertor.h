#pragma once

#include "utfcvutils.h"
#include <tuple>

namespace {

    /* anonymous namespace for translating char types to types defined by utfcvutils.h */

template<int SIZE> struct chartypes { };
template<> struct chartypes<1> { typedef utf8char_t  CHAR; };
template<> struct chartypes<2> { typedef utf16char_t CHAR; };
template<> struct chartypes<4> { typedef utf32char_t CHAR; };

template<int SIZE> struct unsignedforsize { };
template<> struct unsignedforsize<1> { typedef uint8_t  type; };
template<> struct unsignedforsize<2> { typedef uint16_t type; };
template<> struct unsignedforsize<4> { typedef uint32_t type; };
template<> struct unsignedforsize<8> { typedef uint64_t type; };

/* basically stringcopy, taking care of terminating NUL, returning used items */
template<typename D, typename S>
auto identityconvert(S src, S send, D dst, D dend)
{
    while (src<send && dst<dend)
        *dst++ = *src++;
    return std::make_tuple(src, dst);
}
}


/* char_cast converts any integer type 'T' to the correct char type of the same byte-size, as required by the utfcvutils functions */
template<typename T>
const typename chartypes<sizeof(T)>::CHAR* char_cast(const T *p)
{
    typedef const typename chartypes<sizeof(T)>::CHAR* result_type;
    return reinterpret_cast<result_type>(p);
}
template<typename T>
typename chartypes<sizeof(T)>::CHAR* char_cast(T *p)
{
    typedef typename chartypes<sizeof(T)>::CHAR* result_type;
    return reinterpret_cast<result_type>(p);
}

/* cast_to_char converts to the correct char type, given the number of bytes 'TO' needed in the type */
template<size_t TO, typename T>
const typename chartypes<TO>::CHAR* cast_to_char(const T *p)
{
    typedef const typename chartypes<TO>::CHAR* result_type;
    return reinterpret_cast<result_type>(p);
}

/* utfconvertor template converts from utf<FROM> to utf<TO> encoding
 *
 * it has two static functions:
 *     convert  : converts utf<FROM> codepoints from <src> in to utf<TO> codepoints in <dst>
 *                max <maxsize> codeunits are written to <dst> including the terminating NUL.
 *                The number of codeunits used from <src> is returned.
 *     maxsize  : gives a quick calculation of the maximum possible number of codeunits required
 *                for converting <from> utf<FROM> codeunits.
 */
template<int FROM, int TO>
struct utfconvertor {
    // [ sused, dused ] convert(*src, *dst, maxsize);
    // size_t maxsize(size_t from)
};

template<>
struct utfconvertor<1,2> {  enum { FROM=1, TO=2 };
    template<typename D, typename S>
    static auto convert(S src, S send, D dst, D dend) { return utf8toutf16(src, send, dst, dend); }
    template<typename S>
    static auto countneeded(S src, S end) { return utf8toutf16needed(src, end); }

    static size_t maxsize(size_t from) { return from; }
};
template<>
struct utfconvertor<2,1> {  enum { FROM=2, TO=1 };
    template<typename D, typename S>
    static auto convert(S src, S send, D dst, D dend) { return utf16toutf8(src, send, dst, dend); }
    template<typename S>
    static auto countneeded(S src, S end) { return utf16toutf8needed(src, end); }

    // for values between 0x800 and 0xffff you need 3 bytes in utf-8, and only 2 in utf-16
    static size_t maxsize(size_t from) { return 3*from; }
};

template<>
struct utfconvertor<1,4> {  enum { FROM=1, TO=4 };
    template<typename D, typename S>
    static auto convert(S src, S send, D dst, D dend) { return utf8toutf32(src, send, dst, dend); }
    template<typename S>
    static auto countneeded(S src, S end) { return utf8toutf32needed(src, end); }

    // values below 0x80 require 4 times more size in utf-32
    static size_t maxsize(size_t from) { return from; }
};
template<>
struct utfconvertor<4,1> {  enum { FROM=4, TO=1 };
    template<typename D, typename S>
    static auto convert(S src, S send, D dst, D dend) { return utf32toutf8(src, send, dst, dend); }
    template<typename S>
    static auto countneeded(S src, S end) { return utf32toutf8needed(src, end); }

    // values above 0x10000 require 4 utf-8 bytes
    static size_t maxsize(size_t from) { return 4*from; }
};
template<>
struct utfconvertor<4,2> {  enum { FROM=4, TO=2 };
    template<typename D, typename S>
    static auto convert(S src, S send, D dst, D dend) { return utf32toutf16(src, send, dst, dend); }
    template<typename S>
    static auto countneeded(S src, S end) { return utf32toutf16needed(src, end); }

    // values above 0x10000 require 2 utf-16 bytes
    static size_t maxsize(size_t from) { return 2*from; }
};
template<>
struct utfconvertor<2,4> {  enum { FROM=2, TO=4 };
    template<typename D, typename S>
    static auto convert(S src, S send, D dst, D dend) { return utf16toutf32(src, send, dst, dend); }
    template<typename S>
    static auto countneeded(S src, S end) { return utf16toutf32needed(src, end); }

    static size_t maxsize(size_t from) { return from; }
};
template<>
struct utfconvertor<1,1> {  enum { FROM=1, TO=1 };
    template<typename D, typename S>
    static auto convert(S src, S send, D dst, D dend) { return identityconvert(src, send, dst, dend); }
    template<typename S>
    static auto countneeded(S src, S end) { return std::distance(src, end); }

    static size_t maxsize(size_t from) { return from; }
};
template<>
struct utfconvertor<2,2> {  enum { FROM=2, TO=2 };
    template<typename D, typename S>
    static auto convert(S src, S send, D dst, D dend) { return identityconvert(src, send, dst, dend); }
    template<typename S>
    static auto countneeded(S src, S end) { return std::distance(src, end); }

    static size_t maxsize(size_t from) { return from; }
};
template<>
struct utfconvertor<4,4> {  enum { FROM=4, TO=4 };
    template<typename D, typename S>
    static auto convert(S src, S send, D dst, D dend) { return identityconvert(src, send, dst, dend); }
    template<typename S>
    static auto countneeded(S src, S end) { return std::distance(src, end); }

    static size_t maxsize(size_t from) { return from; }
};



