#pragma once
#include <utility>
#include <iterator>
#include <algorithm>
#include "string-base.h"

/*
 * converts strings to numbers.
 *
 * Four ways to specify a string:
 *    * a pair of pointers or iterators
 *    * a pointer to a NUL terminated string
 *    * a pointer and a size
 *    * a container supporting std::begin and std::end 
 *
 * `parseunsigned` and `parsesigned` return both the result, and the
 * position where parsing stopped.
 *
 * `string_to_unsigned` returns only the result, and throws when
 * parsing stopped before the end of the string.
 *
 * `hex2binary`  converts a bound string of hex values to a vector of those values.
 *
 * Two variants:
 *   * convert a bound string to a bound value range,
 *     returning the number of converted values.
 *   * taking a string, returning a specified vector type.
 *
 */


/*
 * convert a ascii digit to a number, for any base up to 36.
 *
 * digits are taken to be: 0..9, a..z 
 *    or 0..9, A..Z
 */
template<typename T>
int char2nyble(T c)
{
    if (c<'0') return -1;
    if (c<='9') return c-'0';
    if (c<'A') return -1;
    if (c<='Z') return 10+c-'A';
    if (c<'a') return -1;
    if (c<='z') return 10+c-'a';
    return -1;
}

/*
    parses a string bound by a pointer pair.

    failure is indicated by returning 'first'.
 */
template<typename P>
std::pair<uint64_t, P> parseunsigned(P first, P last, int base)
{
    int state = 0;    // 3 = foundbase, 2 = invalid, 1 = found '0', 0 = initial state.
    int digits = 0;
    uint64_t num = 0;
    auto p = first;
    while (p<last)
    {
        int n = char2nyble(*p);
        if (base==0) {
            if (state==0) {
                if (1<=n && n<=9) {
                    base = 10;
                    state = 3;
                    num = n;  // this is the first real digit
                    digits = 1;
                }
                else if (n==0) {
                    // expect 0<octal>, 0b<binary>, 0x<hex>
                    state = 1;
                }
                else {
                    // invalid
                    state = 2;
                    break;
                }
            }
            else if (state==1) {
                if (*p == 'x') {
                    base = 16;
                    state = 3;
                }
                else if (*p == 'b') {
                    base = 2;
                    state = 3;
                }
                else if (n==-1) {
                    // just a single '0' followed by a non digit
                    // -> return '0'
                    break;
                }
                else if (0<=n && n<=7) {
                    base = 8;
                    state = 3;
                    num = n;  // this is the first real digit
                    digits = 1;
                }
                else if (n>=8) {
                    // invalid octal digit
                    state = 2;
                    break;
                }
            }
            else {
                // invalid
                state = 2;
                break;
            }
        }
        else {
            if (n<0 || n>=base) {
                // end of number
                break;
            }
            num *= base;
            num += n;
            digits++;
        }

        ++p;
    }
    if (state==2)  // invalid
        return std::make_pair(0, first);

    if (state==3 && digits==0) {
        // "0x" and "0b" are invalid numbers, they expect digits.
        return std::make_pair(0, first);
    }
    return std::make_pair(num, p);
}

/* parses a string bound by a pointer pair. */
template<typename P>
std::pair<int64_t, P> parsesigned(P first, P last, int base)
{
    auto p = first;
    bool negative = false;
    if (first==last)
        return std::make_pair(0, first);
    if (*p == '-') {
        negative = true;
        ++p;
    }
    auto res = parseunsigned(p, last, base);
    if (res.second == p)
        return std::make_pair(0, first);

    uint64_t num = negative ? -res.first : res.first;

    return std::make_pair(num, res.second);
}

/* parses a NUL terminated string */
template<typename T>
std::pair<uint64_t, const T*> parseunsigned(const T* str, int base)
{
    return parseunsigned(str, str+stringlength(str), base);
}


/* parses a NUL terminated string */
template<typename T>
std::pair<int64_t, const T*> parsesigned(const T* str, int base)
{
    return parsesigned(str, str+stringlength(str), base);
}

/* parses a container type string */
template<typename T>
std::pair<uint64_t, typename T::const_iterator> parseunsigned(const T& str, int base)
{
    return parseunsigned(std::begin(str), std::end(str), base);
}

/* parses a container type string */
template<typename T>
std::pair<int64_t, typename T::const_iterator> parsesigned(const T& str, int base)
{
    return parsesigned(std::begin(str), std::end(str), base);
}

template<typename P1,typename P2>
size_t hex2binary(P1 strfirst, P1 strlast, P2 first, P2 last)
{
    typedef typename std::iterator_traits<P2>::value_type value_type;
    int shift = 0;
    value_type word=0;
    auto o= first;
    for (auto i= strfirst ; i!=strlast && o!=last ; i++)
    {
        int n= char2nyble(*i);
        if (n>=0 && n<16) {
            if (shift==0) {
                word= value_type(n);
                shift += 4;
            }
            else {
                word <<= 4;
                word |= n;
                shift += 4;
            }
            if (shift==sizeof(word)*8) {
                *o++ = word;

                shift = 0;
            }
        }
    }
    return o-first;
}

template<typename V, typename S>
auto hex2binary(const S& hexstr)
{
    if constexpr (std::is_pointer_v<S> || std::is_array_v<S>) {
        auto len = stringlength(hexstr);
        V v(len / 2);
        size_t n = hex2binary(hexstr, hexstr+len, v.begin(), v.end());

        // todo: when 'V' is an std::array, don't resize.
        v.resize(n);

        return v;
    }
    else {
        V v(hexstr.size() / 2);
        size_t n = hex2binary(hexstr.begin(), hexstr.end(), v.begin(), v.end());

        // todo: when 'V' is an std::array, don't resize.
        v.resize(n);

        return v;
    }
}
template<typename V, typename P>
auto hex2binary(const P first, const P last)
{
    V v(std::distance(first, last) / 2);
    size_t n = hex2binary(first, last, v.begin(), v.end());
    v.resize(n);

    return v;
}

/* parses a string bound by a pointer pair. */
template<typename P>
uint64_t string_to_unsigned(P first, P last, int base)
{
    auto [ value, end ] = parseunsigned(first, last, base);
    if (last != end)
        throw std::runtime_error("parsing error");
    return value;
}

/* parses a NUL terminated string */
template<typename T>
uint64_t string_to_unsigned(const T* str, int base)
{
    auto [ value, end ] = parseunsigned(str, base);
    if (*end)
        throw std::runtime_error("parsing error");
    return value;
}

/* parses a container type string */
template<typename T>
uint64_t string_to_unsigned(const T& str, int base)
{
    auto [ value, end ] = parseunsigned(str, base);
    if (std::end(str) != end)
        throw std::runtime_error("parsing error");
    return value;
}


/* parses a string bound by a pointer pair. */
template<typename P>
int64_t string_to_signed(P first, P last, int base)
{
    auto [ value, end ] = parsesigned(first, last, base);
    if (last != end)
        throw std::runtime_error("parsing error");
    return value;
}

/* parses a NUL terminated string */
template<typename T>
int64_t string_to_signed(const T* str, int base)
{
    auto [ value, end ] = parsesigned(str, base);
    if (*end)
        throw std::runtime_error("parsing error");
    return value;
}

/* parses a container type string */
template<typename T>
int64_t string_to_signed(const T& str, int base)
{
    auto [ value, end ] = parsesigned(str, base);
    if (std::end(str) != end)
        throw std::runtime_error("parsing error");
    return value;
}

// todo: parse_double
