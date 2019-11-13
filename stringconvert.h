#pragma once
/*
 * Collection of functions for converting strings between the various UTF-XX encodings.
 *
 * The encoding is based on the character size, char, uint8_t and other 8-bit values are
 * assumed to be UTF-8 encoded,
 * short, uint16_t, and other 16-bit values, in UTF-16.
 * long, uint32_t and other 32-bit values in UTF-32.
 *
 * The endianness / byte order is always the compiler's and platform's native order.
 *
 * the conversion result is always a suitable std::string variant.
 *
 *
 * Usage:
 *
 *     std::string utf8 = "utf8 text";
 *     char u8[8];
 *
 *     auto utf16 = string::convert<short>>(utf8);
 *     auto u16a = string::convert<short>(u8, 8);
 *     auto u16b = string::convert<short>(u8, u8+8);
 *
 *
 * (C) 2016  Willem Hengeveld <itsme@xs4all.nl>
 *
 */
#include <string>
#include "utfconvertor.h"

namespace string {

namespace z {
/////////////////////////////////////////////////////////
// z-string utilities, for c style NUL terminated strings

/////////////////////////////////////////////////////////
// string::z::length is basically a templated version of the libc strlen / wcslen functions.
template<typename T>
size_t length(const T *str)
{
    size_t len=0;
    while (*str++)
        len++;
    return len;
}

}


/**
 * should take any kind of container: string, vector, array or stringview
 */
template<typename DSTCHAR, typename SRC>
inline auto convert(const SRC& src)
{
    using SRCCHAR = typename SRC::value_type;

    std::basic_string<DSTCHAR> dst(utfconvertor<sizeof(SRCCHAR), sizeof(DSTCHAR)>::maxsize(std::size(src)), DSTCHAR(0));

    auto [ sused, dused ] = utfconvertor<sizeof(SRCCHAR), sizeof(DSTCHAR)>::convert(std::begin(src), std::end(src), std::begin(dst), std::end(dst));

    dst.erase(dused, dst.end());
    return dst;
}

/**
 *  takes a NUL terminated C type string.
 */
template<typename DSTCHAR, typename SRC>
inline auto convert(const SRC* src)
{
    auto l = z::length(src);
    std::basic_string<DSTCHAR> dst(utfconvertor<sizeof(SRC), sizeof(DSTCHAR)>::maxsize(l), DSTCHAR(0));

    auto [ sused, dused ] = utfconvertor<sizeof(SRC), sizeof(DSTCHAR)>::convert(src, src + l, std::begin(dst), std::end(dst));

    dst.erase(dused, dst.end());
    return dst;
}
/**
 *  takes a pointer and a length
 */
template<typename DSTCHAR, typename SRC>
inline auto convert(const SRC* src, size_t length)
{
    std::basic_string<DSTCHAR> dst(utfconvertor<sizeof(SRC), sizeof(DSTCHAR)>::maxsize(length), DSTCHAR(0));

    auto [ sused, dused ] = utfconvertor<sizeof(SRC), sizeof(DSTCHAR)>::convert(src, src + length, std::begin(dst), std::end(dst));

    dst.erase(dused, dst.end());
    return dst;
}

/**
 *  takes iterator or pointer pair.
 */
template<typename DSTCHAR, typename SRCPTR>
inline auto convert(SRCPTR first, SRCPTR last)
{
    using SRCCHAR = typename std::iterator_traits<SRCPTR>::value_type;

    std::basic_string<DSTCHAR> dst(utfconvertor<sizeof(SRCCHAR), sizeof(DSTCHAR)>::maxsize(std::distance(first, last)), DSTCHAR(0));

    auto [ sused, dused ] = utfconvertor<sizeof(SRCCHAR), sizeof(DSTCHAR)>::convert(first, last, std::begin(dst), std::end(dst));

    dst.erase(dused, dst.end());
    return dst;
}

#ifdef _WIN32
#ifdef __cplusplus_winrt 
/////////////////////////////////////////////////////////
// convert C# platform string
template<typename DST>
inline std::basic_string<DST> convert(Platform::String^ps)
{
    std::basic_string<wchar_t> src;
    for (wchar_t wc : ps)
        src += wc;
    return string::convert<DST>(src);
}
#endif
#endif

} // namespace
