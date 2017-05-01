#pragma once

namespace {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#include "utfcvutils.cpp"
#pragma clang diagnostic pop

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
size_t identityconvert(const S* src, D* dst, size_t maxsize)
{
    D* p= dst;
    D* pend= dst+maxsize-1;
    while (*src && p<pend)
        *p++ = *src++;
    *p = 0;
    return p-dst;
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
    // size_t convert(*src, *dst, maxsize);
    // size_t maxsize(size_t from)
};

template<>
struct utfconvertor<1,2> {  enum { FROM=1, TO=2 };
    template<typename SRC, typename DST>
    static size_t convert(const SRC *src, DST *dst, size_t maxsize) { return utf8toutf16(char_cast(src), char_cast(dst), maxsize); }

    static size_t maxsize(size_t from) { return from; }
};
template<>
struct utfconvertor<2,1> {  enum { FROM=2, TO=1 };
    template<typename SRC, typename DST>
    static size_t convert(const SRC *src, DST *dst, size_t maxsize) { return utf16toutf8(char_cast(src), char_cast(dst), maxsize); }

    // for values between 0x800 and 0xffff you need 3 bytes in utf-8, and only 2 in utf-16
    static size_t maxsize(size_t from) { return 3*from; }
};

template<>
struct utfconvertor<1,4> {  enum { FROM=1, TO=4 };
    template<typename SRC, typename DST>
    static size_t convert(const SRC *src, DST *dst, size_t maxsize) { return utf8toutf32(char_cast(src), char_cast(dst), maxsize); }

    // values below 0x80 require 4 times more size in utf-32
    static size_t maxsize(size_t from) { return from; }
};
template<>
struct utfconvertor<4,1> {  enum { FROM=4, TO=1 };
    template<typename SRC, typename DST>
    static size_t convert(const SRC *src, DST *dst, size_t maxsize) { return utf32toutf8(char_cast(src), char_cast(dst), maxsize); }

    // values above 0x10000 require 4 utf-8 bytes
    static size_t maxsize(size_t from) { return 4*from; }
};
template<>
struct utfconvertor<4,2> {  enum { FROM=4, TO=2 };
    template<typename SRC, typename DST>
    static size_t convert(const SRC *src, DST *dst, size_t maxsize) { return utf32toutf16(char_cast(src), char_cast(dst), maxsize); }

    // values above 0x10000 require 2 utf-16 bytes
    static size_t maxsize(size_t from) { return 2*from; }
};
template<>
struct utfconvertor<2,4> {  enum { FROM=2, TO=4 };
    template<typename SRC, typename DST>
    static size_t convert(const SRC *src, DST *dst, size_t maxsize) { return utf16toutf32(char_cast(src), char_cast(dst), maxsize); }

    static size_t maxsize(size_t from) { return from; }
};
template<>
struct utfconvertor<1,1> {  enum { FROM=1, TO=1 };
    template<typename SRC, typename DST>
    static size_t convert(const SRC *src, DST *dst, size_t maxsize) { return identityconvert(src, dst, maxsize); }
    static size_t maxsize(size_t from) { return from; }
};
template<>
struct utfconvertor<2,2> {  enum { FROM=2, TO=2 };
    template<typename SRC, typename DST>
    static size_t convert(const SRC *src, DST *dst, size_t maxsize) { return identityconvert(src, dst, maxsize); }
    static size_t maxsize(size_t from) { return from; }
};
template<>
struct utfconvertor<4,4> {  enum { FROM=4, TO=4 };
    template<typename SRC, typename DST>
    static size_t convert(const SRC *src, DST *dst, size_t maxsize) { return identityconvert(src, dst, maxsize); }
    static size_t maxsize(size_t from) { return from; }
};


