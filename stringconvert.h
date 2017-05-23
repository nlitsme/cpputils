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
 * (C) 2016  Willem Hengeveld <itsme@xs4all.nl>
 *
 */

// fixed to UTFCV
#define USE_UTFCV 1


// stringconver can use three different libraries for the actual conversions:
//    boost/locale.hpp, libstdc++ codecvt, or my own utfconvertor.h

#ifdef USE_BOOST_LOCALE
//  adds dependency on boost-locale lib
#include <boost/locale.hpp>

#elif defined(USE_CODECVT)
// does not work properly - due to lack of codecvt_utf8<T,T> templates
// see http://www.italiancpp.org/2016/04/20/unicode-localization-and-cpp-support/
// note: currently doing  SRC -> utf8 -> DST, instead of directly converting from SRC to DST.
#include <codecvt>   // codecvt_utf8
#include <locale>    // wstring_convert

#elif defined(USE_UTFCV)
#include "utfconvertor.h"
#endif

#include <string>

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


/////////////////////////////////////////////////////////
// convert std::string type strings
template<typename DST, typename SRC>
inline std::basic_string<DST> convert(const std::basic_string<SRC>& src)
{
#ifdef USE_BOOST_LOCALE
    return boost::locale::conv::utf_to_utf<DST>(src);
#elif defined(USE_CODECVT)
    // note: problem - codecvt_utf8<char> is missing
    std::wstring_convert<std::codecvt_utf8<SRC>,SRC> fromsrc;// widestring<SRC> -> char
    std::wstring_convert<std::codecvt_utf8<DST>,DST> todst;  // char -> widestring<DST>

    return todst.from_bytes(fromsrc.to_bytes(src));
#elif defined(USE_UTFCV)
    std::basic_string<DST> dst(utfconvertor<sizeof(SRC), sizeof(DST)>::maxsize(src.size())+1, DST(0));

    /*size_t used =*/ utfconvertor<sizeof(SRC), sizeof(DST)>::convert(src.c_str(), &dst[0], dst.size());
    dst.resize(z::length(dst.c_str()));
    return dst;
#endif
}


/////////////////////////////////////////////////////////
// convert C NUL terminated type strings
template<typename DST, typename SRC>
inline std::basic_string<DST> convert(const SRC* src)
{
#ifdef USE_BOOST_LOCALE
    return boost::locale::conv::utf_to_utf<DST>(src);
#elif defined(USE_CODECVT)
    std::wstring_convert<std::codecvt_utf8<SRC>,SRC> fromsrc;// widestring<SRC> -> char
    std::wstring_convert<std::codecvt_utf8<DST>,DST> todst;  // char -> widestring<DST>

    return todst.from_bytes(fromsrc.to_bytes(src));
#elif defined(USE_UTFCV)
    std::basic_string<DST> dst(utfconvertor<sizeof(SRC), sizeof(DST)>::maxsize(z::length(src))+1, DST(0));

    /*size_t used =*/ utfconvertor<sizeof(SRC), sizeof(DST)>::convert(src, &dst[0], dst.size());
    dst.resize(z::length(dst.c_str()));
    return dst;
#endif
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

