#pragma once
/*
 * A string formatter using iostream.
 *
 * most features of the printf format strings are implemented.
 * integer and string size specifiers like  '%ld', '%zs' or '%ls' are ignored.
 *
 * Usage:
 *   std::cout << string::formatter("%d", 123);
 *
 *   or:
 *
 *   print("%d", 123);
 *
 *
 * (C) 2016 Willem Hengeveld <itsme@xs4all.nl>
 */
#include <sstream>
#include <ostream>
#include <string>
#include <cstring>      // strchr
#ifdef _WIN32
#include <windows.h>
#endif
#include <vector>
#include <array>
#include <set>
#include <map>

#include "stringconvert.h"
#include "hexdumper.h"
#include "fhandle.h"

#include "templateutils.h"


/******************************************************************************
 * add StringFormatter support for various types by implementing a operatoer<<:
 *
 *
 *  windows managed c++  Platform::String, Platform::Guid
 *  windows managed c++ Windows::Storage::Streams::IBuffer
 *  windows GUID
 *  Qt  QString
 *  std::vector, std::array, std::set, std::map
 *  non-char std::string ( does unicode conversion )
 *  objects containing a 'ToString' method
 *  P
 */
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
        if constexpr (std::is_integral_v<T> && sizeof(T)==1) {
            os << +b;
        }
        else {
            // todo: why does is_stream_insertable_v not work here,
            // then suddenly all arrays and vectors print as <?>
            os << b;
        }
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
        if constexpr (std::is_integral_v<T> && sizeof(T)==1) {
            os << +b;
        }
        else {
            os << b;
        }
        first= false;
    }
    return os;
}

template<typename T, typename COMP, typename A>
std::ostream& operator<<(std::ostream&os, const std::set<T, COMP, A>& buf)
{
    auto fillchar = os.fill();
    std::ios state(NULL);  state.copyfmt(os);

    bool first= true;
    for (const auto& b : buf) {
        os.copyfmt(state);
        if (!first && fillchar) os << fillchar;
        if constexpr (std::is_integral_v<T> && sizeof(T)==1) {
            os << +b;
        }
        else {
            os << b;
        }
        first= false;
    }
    return os;
}

template<typename K, typename V, typename COMP, typename A>
std::ostream& operator<<(std::ostream&os, const std::map<K, V, COMP, A>& buf)
{
    auto fillchar = os.fill();
    std::ios state(NULL);  state.copyfmt(os);

    bool first= true;
    for (const auto& kv : buf) {
        os.copyfmt(state);
        if (!first && fillchar) os << fillchar;
        if constexpr (std::is_integral_v<K> && sizeof(K)==1) {
            os << +kv.first;
        }
        else {
            os << kv.first;
        }

        os << ':';

        if constexpr (std::is_integral_v<V> && sizeof(V)==1) {
            os << +kv.second;
        }
        else {
            os << kv.second;
        }
        first= false;
    }
    return os;
}

#ifdef __SIZEOF_INT128__
template<typename NUM>
std::enable_if_t<std::is_same_v<NUM,__int128_t>,std::ostream&> operator<<(std::ostream&os, NUM num)
{
    if (num == 0)
        return os << '0';

    std::string txt(40, char(0));
    auto p = txt.end();

    bool isneg = false;
    __uint128_t unum;
    if (num < 0) {
        isneg = true;
        unum = -num;
    }
    else {
        unum = num;
    }

    while (unum) {
        int digit = unum % 10;
        unum /= 10;
        *--p = '0' + digit;
    }
    if (isneg)
        *--p = '-';

    txt.erase(txt.begin(), p);

    return os << txt;
}
template<typename NUM>
std::enable_if_t<std::is_same_v<NUM,__uint128_t>,std::ostream&> operator<<(std::ostream&os, NUM num)
{
    if (num == 0)
        return os << '0';

    std::string txt(40, char(0));
    auto p = txt.end();

    while (num) {
        int digit = num % 10;
        num /= 10;
        *--p = '0' + digit;
    }

    txt.erase(txt.begin(), p);

    return os << txt;
}
#endif

#if 0
namespace {
    template<typename Tuple, std::size_t...N>
    auto extract_tuple(const Tuple& tup, std::index_sequence<N...> )
    {
        return std::make_tuple( std::get<N+1>(tup)... );
    }
    template<typename Tuple>
    auto tuple_tail(const Tuple& tup)
    {
        return extract_tuple(tup, std::make_index_sequence<std::tuple_size_v<Tuple>-1>{} );
    }
}
template<typename T, typename...ARGS>
std::ostream& operator<<(std::ostream&os, const std::tuple<T, ARGS...>& value)
{
    if constexpr (std::is_integral_v<T> && sizeof(T)==1) {
        os << +std::get<0>(value);
    }
    else {
        os << std::get<0>(value);
    }

    if constexpr (sizeof...(ARGS) > 0) {
        os << tuple_tail(value);
    }

    return os;
}
#endif


// convert all none <char> strings to a char string
// before outputting to the stream
template<typename CHAR>
inline std::enable_if_t<!std::is_same_v<CHAR,char>, std::ostream&> operator<<(std::ostream&os, const std::basic_string<CHAR>& s)
{
    return os << string::convert<char>(s);
}
inline std::ostream& operator<<(std::ostream&os, const wchar_t *s)
{
    return os << string::convert<char>(s);
}

/*****************************************************************************
 *
 */

namespace {
/*
 * some utility templates used for unpacking the parameter pack
 */
template<int ...> struct seq {};

template<int N, int ...S> struct gens : gens<N-1, N-1, S...> {};

template<int ...S> struct gens<0, S...>{ typedef seq<S...> type; };

/*
 * some template utilities, used to determine when to use hexdump
 */

/**  test if T is a hexdumper type */
template<typename T>
struct is_hexdumper : std::false_type {};

template<typename T>
struct is_hexdumper<Hex::Hexdumper<T> > : std::true_type {};
template<typename T>
constexpr bool is_hexdumper_v = is_hexdumper<T>::value;

}

/*****************************************************************************
 * the StringFormatter class,
 *
 * keeps it's arguments in a tuple, outputs to a ostream when needed.
 *
 */

template<typename...ARGS>
struct StringFormatter {
    const char *fmt;
    std::tuple<ARGS...> args;

    StringFormatter(const char *fmt, ARGS&&...args)
        : fmt(fmt), args(std::forward<ARGS>(args)...)
    {
    }
    StringFormatter(StringFormatter && f)
        : fmt(f.fmt),  args(std::move(f.args))
    {
    }
    friend std::ostream& operator<<(std::ostream&os, const StringFormatter& o)
    {
        o.tostream(os);
        return os;
    }
    void tostream(std::ostream&os) const
    {
        invokeformat(os, typename gens<sizeof...(ARGS)>::type());
    }
    template<int ...S>
    void invokeformat(std::ostream&os, seq<S...>) const
    {
        format(os, fmt, std::get<S>(args) ...);
    }


    //////////////
    // from here on all methods are static.
    //////////////

    // handle case when no params are left.
    static void format(std::ostream&os, const char *fmt) 
    {
        const char *p= fmt;
        while (*p) {
            if (*p=='%') {
                p++;
                if (*p=='%') {
                    os << *p++;
                }
                else {
                    throw std::runtime_error("not enough arguments to format");
                }
            }
            else {
                os << *p++;
            }
        }
    }
    static void applytype(std::ostream& os, char type)
    {
        // unused type/size chars: b k m n r v w y
        switch(type)
        {
            case 'b': // 'b' for Hex::dumper
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
                throw std::runtime_error("unknown format char");
        }
        if ('A'<=type && type<='Z')
                os << std::uppercase;
        else
                os << std::nouppercase;
    }

    template<typename T, typename...FARGS>
    static void format(std::ostream& os, const char *fmt, T& value, FARGS&&...args) 
    {
        bool used_value = false;
        const char *p= fmt;
        while (*p) {
            if (*p=='%') {
                p++;
                if (*p=='%') {
                    os << *p++;
                }
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
                        //blankforpositive= true;  // <-- todo
                    }
                    
                    // '0' means pad with zero
                    // ',' is useful for arrays
                    char padchar= ' ';

                    if (*p=='0') { p++; padchar='0'; }
                    else if (*p==',') { p++; padchar=','; }

                    // width specification
                    char *q;
                    // todo: support '*'  : take size from argumentlist.
                    // todo: support '#'  : adds 0, 0x, 0X prefix to oct/hex numbers -> 'showbase'
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
                    applytype(os, type);
                    if (forcesign)
                        os << std::showpos;
                    else
                        os << std::noshowpos;

                    if (leftadjust)
                        os << std::left;
                    else if (forcesign) {
                        // exception: when forcing display of sign
                        // we need to use 'internal fill'
                        os << std::internal;
                    }

                    if (havewidth) {
                        os.width(width);
                        if (!leftadjust && !forcesign)
                            os << std::right;
                    }
                    else {
                        os.width(0);
                    }
                    // todo: support precision(truncate) for strings
                    if (haveprecision)
                        os.precision(precision);
                    os.fill(padchar);

                    if (type=='c' && output_wchar(os, value))
                    {
                        // nop
                    }
                    else if (type=='p' && output_pointer(os, value))
                    {
                        // nop
                    }
                    else if (type=='b' && output_hex_data(os, value))
                    {
                        // nop
                    }
                    else if (std::strchr("iduoxX", type) && output_int(os, value))
                    {
                        // nop
                    }
                    else if (output_using_operator(os, value)) {
                        // nop
                    }
                    else {
                        os << "<?>";
                    }

                    // TODO: improve float formatting

                    // reset precision
                    os.precision(0);

                    used_value = true;

                    format(os, p, args...);
                    return;
                }
            }
            else {
                os << *p++;
            }
        }

        if (!used_value)
            throw std::runtime_error("too many arguments for format");
    }
    // we need to distinguish real pointers from other types.
    // otherwise the compiler would fail when trying to 
    // cast a non-pointer T to const void*
    template<typename T>
    static bool output_pointer(std::ostream& os, const T& value)
    { 
        if constexpr (std::is_pointer_v<T>) {
            os << (const void*)value;
            return true;
        }
        return false;
    }
    template<typename T>
    static bool output_using_operator(std::ostream& os, const T& value)
    {
        if constexpr (is_stream_insertable_v<T>) {
            os << value;
            return true;
        }
        return false;
    }

    // make sure we don't call string::convert with non char types.
    template<typename T>
    static bool output_wchar(std::ostream& os, const T& value)
    { 
        if constexpr (std::is_integral_v<T>) {
            std::basic_string<wchar_t> wc(1, wchar_t(value));
            os << string::convert<char>(wc);
            return true;
        }
        return false;
    }

    // make sure we call Hex::dumper only for bytevectors or arrays
    template<typename T>
    static bool output_hex_data(std::ostream& os, const T& value)
    { 
        if constexpr (is_hexdumper_v<T>) {
            if (os.fill()=='0')
                os.fill(0);
            os << std::hex << value;
            return true;
        }
        else if constexpr (is_container_v<T>) {
            if constexpr (std::is_integral_v<typename T::value_type>) {
                if (os.fill()=='0')
                    os.fill(0);
                os << std::hex << Hex::dumper(value);
                return true;
            }
        }
        return false;
    }

    template<typename T>
    static bool output_int(std::ostream& os, T value)
    {
        if constexpr ((std::is_integral_v<T> && sizeof(T) == 1) || std::is_same_v<T, wchar_t>) {
            os << (unsigned long)value;
            return true;
        }
        else if constexpr (std::is_integral_v<T> || std::is_floating_point_v<T>) {
            os << value;
            return true;
        }

        return false;
    }

};

namespace string {
template<typename...ARGS>
auto formatter(const char *fmt, ARGS&&...args)
{
    return StringFormatter<ARGS...>(fmt, std::forward<ARGS>(args)...);
}
}

template<typename...ARGS>
std::string stringformat(const char *fmt, ARGS&&...args)
{
    std::stringstream buf;
    buf << StringFormatter<ARGS...>(fmt, std::forward<ARGS>(args)...);

    return buf.str();
}
template<typename...ARGS>
int fprint(FILE *out, const char *fmt, ARGS&&...args)
{
    auto str = stringformat(fmt, std::forward<ARGS>(args)...);
    return fwrite(str.c_str(), str.size(), 1, out);
}
template<typename...ARGS>
int fprint(filehandle out, const char *fmt, ARGS&&...args)
{
    auto str = stringformat(fmt, std::forward<ARGS>(args)...);
    return out.write(str.c_str(), str.size());
}
template<typename...ARGS>
int print(const char *fmt, ARGS&&...args)
{
    return fprint(stdout, fmt, std::forward<ARGS>(args)...);
}

#ifdef QT_VERSION
template<typename...ARGS>
QString qstringformat(const char *fmt, ARGS&&...args)
{
    return QString::fromStdString(stringformat(fmt, std::forward<ARGS>(args)...));
}
#endif

#ifdef _WIN32
template<typename...ARGS>
void debug(const char *fmt, ARGS&&...args)
{
    OutputDebugString(string::convert<TCHAR>(stringformat(fmt, std::forward<ARGS>(args)...).c_str()));
}
#ifdef __cplusplus_winrt 
Platform::String^ ToString()
{
    //String^ str = ref new String();
    return ref new Platform::String(string::convert<wchar_t>(stringformat(fmt, std::forward<ARGS>(args)...)).c_str());
    /*
    IBuffer^ ibuf= WindowsRuntimeBufferExtensions(buf.str().c_str());
    return Windows::Security::Cryptography::CryptographicBuffer::ConvertBinaryToString(BinaryStringEncoding::Utf8, ibuf);
    */
    //return ref new Platform::String(buf.str().c_str(), buf.str().size());
    
}
#endif
#endif


