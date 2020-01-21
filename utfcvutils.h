#include <stddef.h>
#include <stdint.h>
#include <tuple>

// relevant rfcs:
//  rfc 2781  utf16
//  rfc 3629  utf8
//  rfc 2152  utf7      
//  rfc 2279  utf8 (obsolete)
//  rfc 2044  utf8 (obsolete)
//  rfc 1642  utf7 (obsolete)
//  rfc 4042  utf9+utf18 (april 1st)
// todo: remember start of current input symbol, so we can return the 
// correct nr of items used when the output buf is too small
//
// todo: add interface to return status, used-input, used-output

// utf8                                                                                      utf16                                                   utf32
// 00000000-0000007F | 0xxxxxxx                            | 00-7f                         | 00xx         | 00000000 0xxxxxxx                       | 0000 00xx    | 00000000 00000000 00000000 0xxxxxxx 
// 00000080-000007FF | 110xxxxx 10xxxxxx                   | c0-df 80-bf                   | 0xxx         | 00000xxx xxxxxxxx                       | 0000 0xxx    | 00000000 00000000 00000xxx xxxxxxxx
// 00000800-0000FFFF | 1110xxxx 10xxxxxx 10xxxxxx          | e0-ef 80-bf 80-bf             | xxxx         | xxxxxxxx xxxxxxxx                       | 0000 xxxx    | 00000000 00000000 xxxxxxxx xxxxxxxx
// 00010000-0010FFFF | 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx | f0-f7 80-bf 80-bf 80-bf       | d8yy dcxx    | 110110yy yyyyyyyy  110111xx xxxxxxxx    | d8yy dcxx    | 00000000 0001xxxx xxxxxxxx xxxxxxxx
//
// the obsolete rfc 2279 also specified these ranges:
// 00200000-03FFFFFF | 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
// 04000000-7FFFFFFF | 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx

// utf16
// d800-dbff   110110yy yyyyyyyy  high half
// dc00-dfff   110111xx xxxxxxxx  low half
//    ->  yyyy yyyyyyxx xxxxxxxx   + 0x10000



/////////////////////////////////////////////////////////
//  <UTFNN>to<UTFNN>bytesneeded: calculate how many bytes exactly a conversion will need
//
//  <UTFNN>charcount:            calculate how many symbols there are in a string
//
//  <UTFNN>to<UTFNN>             convert between utf representations, return nr of code points used.
//                               result is terminated with NUL symbol.
//

/*
 *  todo:
 *  add failure policy argument:
 *     - ignore  -> ignores bad code points
 *     - copy    -> when possible just copy the bad code points
 *     - replace -> replace bad code points with '?'
 *     - throw   -> throw an exception
 *     - abort   -> return truncated string
 *
 * todo: added 'last' argument to *bytesneeded functions, so we can support non-NUL terminated strings.
 * todo: added 'last' argument to *convert functions, so we can support non-NUL terminated strings.
 *
 */
typedef uint8_t utf8char_t;
typedef uint16_t utf16char_t;
typedef uint32_t utf32char_t;


// =========
//  count 
// =========
namespace {
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#endif
size_t utf32toutf8bytesneeded(utf32char_t c)
{
    if (c<0x80) return 1;
    if (c<0x800) return 2;
    if (c<0x10000) return 3;
    return 4; // < 0x110000
}
#ifdef __clang__
#pragma clang diagnostic pop
#endif
}
template<typename P>
size_t utf32toutf8bytesneeded(P p, P pend)
{
    size_t n=0;
    while (p < pend)
        n += utf32toutf8bytesneeded(*p++);
    return n;
}
template<typename P>
size_t utf16toutf8bytesneeded(P p, P pend)
{
    size_t n=0;
    utf32char_t w=0;
    while (p < pend) {
        uint16_t c = *p++;
        if (c<0xd800 || c>=0xe000)
            n += utf32toutf8bytesneeded(c);
        else if (c<0xdc00) {
            w = c&0x3ff;
        }
        else { // dc00..e000
            w = 0x10000 + ((w<<10) | (c&0x3ff));
            n += utf32toutf8bytesneeded(w);
            w = 0;
        }
    }
    return n;
}

template<typename P>
size_t utf8charcount(P p, P pend)
{
    size_t n=0;
    while (p < pend) {
        uint8_t c= *p++;
        if (c<0x80 || c>=0xc0)
            n++;
    }
    return n;
}
template<typename P>
size_t utf8toutf32bytesneeded(P p, P pend)
{
    return utf8charcount(p, pend)*sizeof(utf32char_t);
}
template<typename P>
size_t utf8toutf16bytesneeded(P p, P pend)
{
    size_t n=0;
    while (p < pend) {
        uint8_t c= *p++;
        if (c<0x80 || c>=0xc0)
            n++;
        if (c>=0xf0)
            n++;
    }
    return n*sizeof(utf16char_t);
}
template<typename P>
size_t utf32charcount(P p, P pend)
{
    return pend - p;
}
template<typename P>
size_t utf32toutf16bytesneeded(P p, P pend)
{
    size_t n=0;
    while (p < pend) {
        utf32char_t c= *p++;
        n++;
        if (c>=0x10000)
            n++;
    }
    return n*sizeof(utf16char_t);
}

// todo: lookup proper name ... utf32 prefix
template<typename P>
size_t utf16charcountxxxx(P p, P pend)
{
    if (p >= pend)
        return 0;
    uint32_t c = *p;
    if (c<0x10000) return 2;
    return 4;   // < 0x110000
}
template<typename P>
size_t utf16bytesneeded(P p, P pend)
{
    size_t n=0;
    while (p < pend)
        n += utf16charcountxxxx(*p++);
    return n;
}
template<typename P>
size_t utf16charcount(P p, P pend)
{
    size_t n=0;
    while (p < pend) {
        uint16_t c= *p++;
        if (c<0xdc00 || c>=0xe000)
            n++;
    }
    return n;
}
template<typename P>
size_t utf16toutf32bytesneeded(P p, P pend)
{
    return utf16charcount(p, pend)*sizeof(utf32char_t);
}

// =========
//  convert
// =========
//
//
// problem with some converts:
//   not enough room in dst, while we did already read several src bytes.
//   in utf8->utf16
//
//   in utf16->utf8

template<typename S, typename D>
auto utf8toutf32(S src, S send, D dst, D dend)
{
    int n=-1;
    utf32char_t w=0;
    while (src < send)
    {
        uint8_t c = *src;
        if (c<0x80) {
            if (dst+1 > dend)
                break;
            *dst++ = c;
        }
        else if (c<0xc0) {
            // break on invalid utf8
            if (n<0)
                break;

            w = (w<<6) | (c&0x3f);
            if (n==0) {

                // break on invalid codes
                if (w>=0xd800 && w<0xe000)
                    break;
                if (w>=0x110000)
                    break;

                if (dst+1 > dend)
                    break;
                *dst++ = w;
                n=-1;
            }
            else
                n--;
        }
        else if (c<0xe0) {
            w = (c&0x1f);
            n= 0;
        }
        else if (c<0xf0) {
            w = (c&0xf);
            n= 1;
        }
        else {  // c < 0xf8
            w = (c&0x7);
            n= 2;
        }
        ++src;
    }
    return std::make_tuple(src, dst);
}
template<typename S, typename D>
auto utf32toutf8(S src, S send, D dst, D dend)
{
    while (src < send)
    {
        uint32_t c = *src;
        if (c<0x80) {
            if (dst+1 > dend)
                break;
            *dst++ = c;
        }
        else if (c<0x800) {
            if (dst+2 > dend)
                break;
            *dst++ = 0xc0 + (c>>6);
            *dst++ = 0x80 + (c&0x3f);
        }
        else if (c<0x10000) {
            // break on invalid codes
            if (c>=0xd800 && c<0xe000)
                break;

            if (dst+3 > dend)
                break;
            *dst++ = 0xe0 + (c>>12);
            *dst++ = 0x80 + ((c>>6)&0x3f);
            *dst++ = 0x80 + (c&0x3f);
        }
        else if (c<0x110000) {
            if (dst+4 > dend)
                break;
            *dst++ = 0xf0 + (c>>18);
            *dst++ = 0x80 + ((c>>12)&0x3f);
            *dst++ = 0x80 + ((c>>6)&0x3f);
            *dst++ = 0x80 + (c&0x3f);
        }
        else {
            // break on invalid codes
            break;
        }

        ++src;
    }
    return std::make_tuple(src, dst);
}

template<typename S, typename D>
auto utf16toutf32(S src, S send, D dst, D dend)
{
    utf32char_t w=0;
    int n=-1;
    while (src < send)
    {
        uint16_t c = *src;
        if (c<0xd800 || c>=0xe000) {
            if (dst+1 > dend)
                break;
            *dst++ = c;
        }
        else if (c<0xdc00) {
            // break on  d8xx d8xx sequences
            if (n!=-1)
                break;

            w = c&0x3ff;
            n = 0;
        }
        else { // c=dc00 .. dfff
            // break on  dcxx  without d8xx
            if (n!=0)
                break;
            w = 0x10000 + ((w<<10) | (c&0x3ff));
            if (dst+1 > dend)
                break;
            *dst++ = w;

            n = -1;
        }

        ++src;
    }
    return std::make_tuple(src, dst);
}
template<typename S, typename D>
auto utf32toutf16(S src, S send, D dst, D dend)
{
    while (src < send)
    {
        uint32_t c = *src;
        if (c<0x10000) {
            // break on invalid codes
            if (c>=0xd800 && c<0xe000)
                break;
            if (dst+1 > dend)
                break;
            *dst++ = c;
        }
        else if (c<=0x110000) {
            if (dst+2 > dend)
                break;
            c -= 0x10000;
            *dst++ = 0xd800+(c>>10);
            *dst++ = 0xdc00+(c&0x3ff);
        }
        else {
            // break on invalid codes
            break;
        }

        ++src;
    }
    return std::make_tuple(src, dst);
}
template<typename S, typename D>
auto utf8toutf16(S src, S send, D dst, D dend)
{
    int n=-1;
    utf32char_t w=0;
    while (src < send)
    {
        uint8_t c = *src;
        if (c<0x80) {
            if (dst+1 > dend)
                break;
            *dst++ = c;
        }
        else if (c<0xc0) {
            // break on invalid utf8
            if (n<0)
                break;

            w = (w<<6) | (c&0x3f);
            if (n==0) {
                // break on invalid codes
                if (w>=0xd800 && w<0xe000)
                    break;
                if (w>=0x110000)
                    break;

                if (w<0x10000) {
                    if (dst+1 > dend)
                        break;
                    *dst++ = w;
                }
                else {
                    if (dst+2 > dend)
                        break;
                    w -= 0x10000;
                    *dst++ = 0xd800+(w>>10);
                    *dst++ = 0xdc00+(w&0x3ff);
                }
                n=-1;
            }
            else
                n--;
        }
        else if (c<0xe0) {
            w = (c&0x1f);
            n= 0;
        }
        else if (c<0xf0) {
            w = (c&0xf);
            n= 1;
        }
        else {  // c < 0xf8
            w = (c&0x7);
            n= 2;
        }

        ++src;
    }
    return std::make_tuple(src, dst);
}
template<typename S, typename D>
auto utf16toutf8(S src, S send, D dst, D dend)
{
    utf32char_t w=0;
    int n=-1;
    bool emit=false;
    while (src < send)
    {
        uint16_t c = *src;
        if ((c<0xd800) || (c>=0xe000)) {
            w = c;
            emit= true;
        }
        else if (c<0xdc00) {
            // break on  d8xx d8xx sequences
            if (n!=-1)
                break;

            w = c&0x3ff;
            emit= false;
            n = 0;
        }
        else { // c=dc00 .. dfff
            // break on  dcxx  without d8xx
            if (n!=0)
                break;

            w = 0x10000 + ((w<<10) | (c&0x3ff));
            emit= true;

            n = -1;
        }
        if (emit) {
            if (w<0x80) {
                if (dst+1 > dend)
                    break;
                *dst++ = w;
            }
            else if (w<0x800) {
                if (dst+2 > dend)
                    break;
                *dst++ = 0xc0 + (w>>6);
                *dst++ = 0x80 + (w&0x3f);
            }
            else if (w<0x10000) {
                if (dst+3 > dend)
                    break;
                *dst++ = 0xe0 + (w>>12);
                *dst++ = 0x80 + ((w>>6)&0x3f);
                *dst++ = 0x80 + (w&0x3f);
            }
            else {
                if (dst+4 > dend)
                    break;
                *dst++ = 0xf0 + (w>>18);
                *dst++ = 0x80 + ((w>>12)&0x3f);
                *dst++ = 0x80 + ((w>>6)&0x3f);
                *dst++ = 0x80 + (w&0x3f);
            }
        }

        // because now a complete conversion was made, only now increment src.
        src++;
    }
    return std::make_tuple(src, dst);
}

// ============
//  getchar
// ============
//
//
template<typename P>
std::tuple<utf32char_t, P> getutf8(P p, P pend)
{
    if (p >= pend)
        throw std::out_of_range("getutf8");
    utf8char_t b0= *p++;
    utf32char_t c;
    int n;
    if (b0>=0xf0) {
        c= b0&7;
        n=3;
    }
    else if (b0>=0xe0) {
        c= b0&15;
        n=2;
    }
    else if (b0>=0xc0) {
        c= b0&31;
        n=1;
    }
    else if (b0>=0x80) {
        throw std::range_error("invalid utf8");
    }
    else {
        return {b0, p};
    }
    if (p+n > pend)
        throw std::out_of_range("getutf8");
    while (n--) {
        c <<= 6;
        utf8char_t b= *p++;
        c |= b&0x3f;
        if ( (b&0xc0)!=0x80 )
            throw std::range_error("invalid utf8");
    }
    return {c, p};
}
template<typename P>
std::tuple<utf32char_t, P> getutf16(P p, P pend)
{
    if (p >= pend)
        throw std::out_of_range("getutf16");
    utf16char_t b0= *p++;
    if (b0>=0xd800 && b0<0xe000) {
        if (p >= pend)
            throw std::out_of_range("getutf16");
        utf16char_t b1= *p++;
        if (b0>=0xdc00)
            throw std::range_error("invalid utf16");
        if (b1<0xdc00 || b1>=0xe000)
            throw std::range_error("invalid utf16");

        auto val= b0 & 0x3ff;
        val<<=10;
        val|= b1 & 0x3ff;
        val+= 0x10000;
        return {val, p};
    }
    return {b0, p};
}
template<typename P>
std::tuple<utf32char_t, P> getutf32(P p, P pend)
{
    if (p >= pend)
        throw std::out_of_range("getutf32");
    auto val = *p++;
    return {val, p};
}

template<typename P>
std::tuple<utf32char_t, P> getutf(P p, P pend)
{
    if constexpr (sizeof(std::iterator_traits<P>::value_type) == 1)
        return getutf8(p, pend);
    if constexpr (sizeof(std::iterator_traits<P>::value_type) == 2)
        return getutf16(p, pend);
    if constexpr (sizeof(std::iterator_traits<P>::value_type) == 4)
        return getutf32(p, pend);
    throw std::domain_error("getutf");
}

// ============
//  comparing 
// ============
template<typename L, typename R>
int simplestringcompare(L l, L lend, R r, R rend)
{
    while (l < lend && r < rend)
    {
        if (*l < *r)
            return -1;
        if (*l > *r)
            return 1;
        l++;
        r++;
    }
    return 0;
}

template<typename L, typename R>
int utfstringcompare(L l, L lend, R r, R rend)
{
    // utf8 and utf32 can be compared using memcmp
    if constexpr (sizeof(std::iterator_traits<L>::value_type) == 1)
        if constexpr (sizeof(std::iterator_traits<R>::value_type) == 1)
            return simplestringcompare(l, lend, r, rend);
    if constexpr (sizeof(std::iterator_traits<L>::value_type) == 4)
        if constexpr (sizeof(std::iterator_traits<R>::value_type) == 4)
            return simplestringcompare(l, lend, r, rend);

    while (l < lend && r < rend)
    {
        auto [ lval, ln ] = getutf(l, lend);
        auto [ rval, rn ] = getutf(r, rend);
        if (lval<rval)
            return -1;
        if (lval>rval)
            return 1;
        l = ln; r = rn;
    }
    return 0;
}

