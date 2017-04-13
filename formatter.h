#pragma once
/*
 * A string formatter using iostream
 *
 * (C) 2016 Willem Hengeveld <itsme@xs4all.nl>
 */
#include <sstream>
#include <string>
#include <cstring>      // strchr
#ifdef _WIN32
#include <windows.h>
#endif
#include <vector>
#include <array>

#include "stringconvert.h"
#include "hexdumper.h"

template<typename T>
struct is_container : std::false_type {};

template<typename T>
struct is_container<std::vector<T> > : std::true_type {};
template<typename T>
struct is_container<std::basic_string<T> > : std::true_type {};
template<typename T, int N>
struct is_container<std::array<T,N> > : std::true_type {};

#ifdef _WIN32
#ifdef __cplusplus_winrt 
inline std::ostream& operator<<(std::ostream&os, Platform::String^s)
{
    return os << string::convert<char>(s);
}

template<typename T>
inline std::ostream& operator<<(std::ostream&os, T^h)
{
    if (h)
        return os << h->ToString();
    return os << "(null)";
}
inline std::ostream& operator<<(std::ostream&os, Platform::Guid *h)
{
    if (h)
        return os << h->ToString();
    return os << "(null)";
}
inline std::ostream& operator<<(std::ostream&os, Windows::Storage::Streams::IBuffer^ buf)
{
    Platform::Array<unsigned char> ^bbuf = nullptr;
    Windows::Security::Cryptography::CryptographicBuffer::CopyToByteArray(buf, &bbuf);

    // hexdump buffer bytes
    bool first= true;
    for (unsigned char b : bbuf) {
        if (!first) os << ' ';
        os << std::hex; os.fill('0'); os.width(2);
        os << b;
        first= false;
    }
    return os;
}

inline std::ostream& operator<<(std::ostream&os, const GUID& guid)
{
    os.fill('0');
    os.width(8); os << std::hex << guid.Data1 << '-';
    os.width(4); os << std::hex << guid.Data2 << '-';
    os.width(4); os << std::hex << guid.Data3 << '-';
    os << "-";
    for (int i=0 ; i<8 ; i++) {
        if (i==2) os << '-';
        os.width(2); os << std::hex << (unsigned)guid.Data4[i];
    }
    return os;
}

#endif
#endif

#ifdef QT_VERSION
inline std::ostream& operator<<(std::ostream&os, const QString& s)
{
    QByteArray a = s.toUtf8();

    os << std::string(a.data(), a.data()+a.size());

    return os;
}
#endif

// note: originally i would always hexdump array or vector of unsigned.
// now there is the 'hexdumper' object for that.
//
// any array / vector will now be output as a sequence of items, formatted according to the current
// outputformat.


template<typename T, size_t N>
std::ostream& operator<<(std::ostream&os, const std::array<T,N>& buf)
{
    auto fillchar = os.fill();
    std::ios state(NULL);  state.copyfmt(os);

    bool first= true;
    for (const auto& b : buf) {
        os.copyfmt(state);
        if (!first && fillchar) os << fillchar;
        os << unsigned(b);
        first= false;
    }
    return os;
}
template<typename T, typename A>
std::ostream& operator<<(std::ostream&os, const std::vector<T, A>& buf)
{
    auto fillchar = os.fill();
    std::ios state(NULL);  state.copyfmt(os);

    bool first= true;
    for (const auto& b : buf) {
        os.copyfmt(state);
        if (!first && fillchar) os << fillchar;
        os << unsigned(b);
        first= false;
    }
    return os;
}


// convert all none <char> strings to a char string
// before outputting to the stream
template<typename CHAR>
inline std::enable_if_t<!std::is_same<CHAR,char>::value, std::ostream&> operator<<(std::ostream&os, const std::basic_string<CHAR>& s)
{
    return os << string::convert<char>(s);
}
inline std::ostream& operator<<(std::ostream&os, const wchar_t *s)
{
    return os << string::convert<char>(s);
}


/*
 *   how to embed hexdumps:
 *       - using special format chars:  format("%{hexdump:spec}", v)
 *       - type dependent:   hexdump when a std::{vector|array}<unsigned-int> passed to '%s'
 *              this is what is currently implemented.
 *       - using wrapper:   format("%s", hexdump(v, ...))
 *              hexdump class has operator<<(ostream, hexdump) function
 *
 *   * all have problem that entire hexdump will be created before outputting.
 *     so not suitable for very large hexdumps.
 *
 */

struct StringFormatter {
    std::stringstream buf;
    template<typename...ARGS>
    StringFormatter(const char *fmt, ARGS...args)
    {
        format(fmt, args...);
    }

    // handle case when no params are left.
    void format(const char *fmt)
    {
        const char *p= fmt;
        while (*p) {
            if (*p=='%') {
                p++;
                if (*p=='%') {
                    buf << *p++;
                }
                else {
                    throw std::runtime_error("not enough arguments to format");
                }
            }
            else {
                buf << *p++;
            }
        }
    }
    void applytype(std::ostream& os, char type)
    {
        // unused type/size chars: b k m n r v w y
        switch(type)
        {
            case 'b': // 'b' for hex::dumper
                // '-':  only hex, otherwise: hex + ascii
                // '0':  no spaces
                break;
            case 'i':
            case 'd':
            case 'u': // unsigned is part of type
                os << std::dec;
                break;
            case 'o':
                os << std::oct;
                break;
            case 'x':
            case 'X':
                os << std::hex;
                break;
            case 'f': // 123.45
            case 'F':
                os << std::fixed;
                break;
            case 'g':  // shortest of 123.45 and 1.23e+2
            case 'G':
                os.unsetf(os.floatfield);
//                os << std::defaultfloat;
                break;
            case 'a':  // hexadecimal floats
            case 'A':
                os.setf(os.fixed | os.scientific, os.floatfield);
//                os << std::hexfloat;
                break;
            case 'e': // 1.23e+2
            case 'E':
                os << std::scientific;
                break;
            case 'c': // char -> need explicit cast
            case 's': // string - from type
                break;
            case 'p': // pointer value - cast to (void*)
                break;
            default:
                throw "unknown format char";
        }
        if ('A'<=type && type<='Z')
                os << std::uppercase;
        else
                os << std::nouppercase;
    }

#if 0
    // NOTE: hex+asc-dump are not yet finished
    template<typename T>
        void ascdump(std::ostream& os, const char *spec, const char *endspec, T& value)
        {
            os << "ascdump(" << std::string(spec, endspec) << ')';
        }
    template<typename T>
        void hexdump(std::ostream& os, const char *spec, const char *endspec, T& value)
        {
            os << "hexdump(" << std::string(spec, endspec) << ')';
        }
#endif
    template<typename T, typename...ARGS>
    void format(const char *fmt, T& value, ARGS...args)
    {
        bool used_value = false;
        const char *p= fmt;
        while (*p) {
            if (*p=='%') {
                p++;
                if (*p=='%') {
                    buf << *p++;
                }
#if 0
                else if (*p=='{') {
                    // extended format spec
                    //    %{hexdump:...}
                    //    %{ascdump:...}
                    //
                    char *q = std::strchr(p, '}');
                    if (q==NULL)
                        throw "invalid %{} format";
                    if (p+8<=q && std::equal(p+1, p+8, "ascdump")) {
                        ascdump(buf, p+8, q, value);
                        p = q+1;
                    }
                    else if (p+8<=q && std::equal(p+1, p+8, "hexdump")) {
                        hexdump(buf, p+8, q, value);
                        p = q+1;
                    }
                    else {
                        throw "unknown %{} format";
                    }
                }
#endif
                else {

                    // '-'  means left adjust
                    bool leftadjust= false;
                    if (*p=='-') {
                        p++;
                        leftadjust= true;
                    }
                    bool forcesign= false;
                    //bool blankforpositive= false;
                    if (*p=='+') {
                        p++;
                        forcesign= true;
                    }
                    else if (*p==' ') {
                        p++;
                        //blankforpositive= true;
                    }
                    
                    // '0' means pad with zero
                    // ',' is useful for arrays
                    char padchar= ' ';

                    if (*p=='0') { p++; padchar='0'; }
                    else if (*p==',') { p++; padchar=','; }

                    // width specification
                    char *q;
                    // todo: support '*'
                    int width= strtol(p, &q, 10);
                    bool havewidth= p!=q;
                    p= q;

                    // precision after '.'
                    int precision= 0;
                    bool haveprecision= false;
                    if (*p=='.') {
                        p++;
                        // todo: support '*'
                        precision= strtol(p, &q, 10);
                        haveprecision= p!=q;
                        p= q;
                    }

                    // ignore argument size field
                    while (std::strchr("qhlLzjt", *p))
                        p++;
                    if (*p=='I') {
                        // microsoft I64 specifier
                        p++;
                        if (p[1]=='6' || p[1]=='4') {
                            p+=2;
                        }
                    }

                    char type= 0;
                    if (*p)
                        type= *p++;

                    // use formatting
                    applytype(buf, type);
                    if (forcesign)
                        buf << std::showpos;
                    else
                        buf << std::noshowpos;

                    if (leftadjust)
                        buf << std::left;

                    if (havewidth) {
                        buf.width(width);
                        if (!leftadjust)
                            buf << std::right;
                    }
                    else {
                        buf.width(0);
                    }
                    // todo: support precision(truncate) for strings
                    if (haveprecision)
                        buf.precision(precision);
                    buf.fill(padchar);


                    // todo: solve problem with (const wchar_t*) L"xxx" argument
                    //       -> currently this is represented as a pointer, instead of a unicode string.
                    if (type=='c')
                        add_wchar(value);
                    else if (type=='p')
                        add_pointer(value);
                    else if (type=='b')
                        hex_dump_data(value);
                    else
                        buf << value;

                    used_value = true;
                    // todo: make sure 'x', 'd' are outputted as integers.

                    // problem with "%b", std::vector<double>{1,2,3}
                    //     this will now be handled by hexdump, while i would like this to use operator<<
                    format(p, args...);
                    return;
                }
            }
            else {
                buf << *p++;
            }
        }

        if (!used_value)
            throw std::runtime_error("too many arguments for format");
    }
    // we need to distinguish real pointers from other types.
    // otherwise the compiler would fail when trying to 
    // cast a non-pointer T to const void*
    template<typename T>
    std::enable_if_t<std::is_pointer<T>::value, void> add_pointer(const T& value) { buf << (const void*)value; }
    template<typename T>
    std::enable_if_t<!std::is_pointer<T>::value, void> add_pointer(const T& value) { }

    // make sure we don't call string::convert with non char types.
    template<typename T>
    std::enable_if_t<std::is_integral<T>::value, void> add_wchar(const T& value) { 
        std::basic_string<wchar_t> wc(1, wchar_t(value));
        buf << string::convert<char>(wc);
    }
    template<typename T>
    std::enable_if_t<!std::is_integral<T>::value, void> add_wchar(const T& value) { }

    // make sure we call hex::dumper only for bytevectors or arrays
    template<typename T>
    std::enable_if_t<!is_container<T>::value,void> hex_dump_data(const T& value) { }
    template<typename T>
    std::enable_if_t<is_container<T>::value && std::is_same<typename T::value_type,double>::value,void> hex_dump_data(const T& value) { }

    template<typename T>
    std::enable_if_t<is_container<T>::value && !std::is_same<typename T::value_type,double>::value,void> hex_dump_data(const T& value) { 
        if (buf.fill()=='0')
            buf.fill(0);
        buf << std::hex << hex::dumper(value);
    }

    std::string str() const
    {
        return buf.str();
    }

#ifdef _WIN32
    void todebug()
    {
        OutputDebugString(string::convert<TCHAR>(buf.str()).c_str());
    }
#ifdef __cplusplus_winrt 
    Platform::String^ ToString()
    {
        //String^ str = ref new String();
        return ref new Platform::String(string::convert<wchar_t>(buf.str()).c_str());
        /*
        IBuffer^ ibuf= WindowsRuntimeBufferExtensions(buf.str().c_str());
        return Windows::Security::Cryptography::CryptographicBuffer::ConvertBinaryToString(BinaryStringEncoding::Utf8, ibuf);
        */
        //return ref new Platform::String(buf.str().c_str(), buf.str().size());
        
    }
#endif
#endif
    int print(FILE *out= stdout)
    {
        auto str= buf.str();
        return fwrite(str.c_str(), 1, str.size(), out);
    }
};

template<typename...ARGS>
int print(const char *fmt, ARGS...args)
{
    return StringFormatter(fmt, args...).print();
}
template<typename...ARGS>
int fprint(FILE *out, const char *fmt, ARGS...args)
{
    return StringFormatter(fmt, args...).print(out);
}
template<typename...ARGS>
std::string stringformat(const char *fmt, ARGS...args)
{
    return StringFormatter(fmt, args...).str();
}
#ifdef QT_VERSION
template<typename...ARGS>
QString qstringformat(const char *fmt, ARGS...args)
{
    return QString::fromStdString(StringFormatter(fmt, args...).str());
}
#endif

