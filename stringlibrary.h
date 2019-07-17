#pragma once

#include <utility>
#include <iterator>
#include <algorithm>

#include "string-strip.h"

// todo: stringjoin


/*
 * Returns the byte length of a NUL terminated string.
 */
template<typename T>
size_t stringlength(const T *str)
{
    size_t len=0;
    while (*str++)
        len++;
    return len;
}

/*
 * case sensitive compare of two NUL terminated strings.
 */
template<typename TA, typename TB>
int stringcompare(const TA *a, const TB *b)
{
    while (*a && *b && *a == *b) 
    {
        a++;
        b++;
    }
    if (*a<*b)
        return -1;
    if (*a>*b)
        return 1;
    return 0;
}

/*
 * Copy a NUL terminated string.
 * Returns a pointer to the last element copied.
 */
template<typename TA, typename TB>
TA *stringcopy(TA *a, const TB *b)
{
    while ((*a++ = *b++)!=0)
        ;
    return a-1;
}

/*
 * case insensitive character compare
 */
template<typename TA, typename TB>
int charicompare(TA a, TB b)
{
    a=(TA)tolower(a);
    b=(TB)tolower(b);
    if (a<b) return -1;
    if (a>b) return 1;
    return 0;
}
/*
 * case sensitive character compare
 */
template<typename TA, typename TB>
int charcompare(TA a, TB b)
{
    if (a<b) return -1;
    if (a>b) return 1;
    return 0;
}
/*
 * case insensitive compare of two NUL terminated strings.
 */
template<class PA, class PB>
int stringicompare(const PA* a, const PB* b)
{
    while (*a && *b && charicompare(*a, *b)==0)
    {
        a++;
        b++;
    }
    return charicompare(*a, *b);
}

/*
 * case insensitive compare of two stl string types.
 *
 * arguments can be anything which has begin and end, like:
 *      vector, array, string, string_view, span, etc.
 */
template<class TA, class TB>
int stringicompare(const TA& a, const TB& b)
{
    auto pa= std::begin(a);
    auto pa_end= std::end(a);

    auto pb= std::begin(b);
    auto pb_end= std::end(b);

    while (pa!=pa_end && pb!=pb_end && charicompare(*pa, *pb)==0)
    {
        pa++;
        pb++;
    }

    if (pa==pa_end && pb==pb_end)
        return 0;
    if (pa==pa_end)
        return -1;
    if (pb==pb_end)
        return 1;
    return charicompare(*pa, *pb);
}
/*
 * case sensitive compare of two stl string types.
 *
 * arguments can be anything which has begin and end, like:
 *      vector, array, string, string_view, span, etc.
 */
template<class TA, class TB>
int stringcompare(const TA& a, const TB& b)
{
    auto pa= std::begin(a);
    auto pa_end= std::end(a);

    auto pb= std::begin(b);
    auto pb_end= std::end(b);

    while (pa!=pa_end && pb!=pb_end && *pa == *pb)
    {
        pa++;
        pb++;
    }

    if (pa==pa_end && pb==pb_end)
        return 0;
    if (pa==pa_end)
        return -1;
    if (pb==pb_end)
        return 1;
    return charcompare(*pa, *pb);
}

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
    if (state==2)
        return std::make_pair(0, first);

    if (state==3 && digits==0) {
        // "0x" and "0b" are invalid numbers.
        return std::make_pair(0, first);
    }
    return std::make_pair(num, p);
}
template<typename P>
std::pair<int64_t, P> parsesigned(P first, P last, int base)
{
    auto p = first;
    bool negative = false;
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

template<typename P>
std::pair<uint64_t, const P*> parseunsigned(const P* str, int base)
{
    return parseunsigned(str, str+stringlength(str), base);
}


template<typename P>
std::pair<int64_t, const P*> parsesigned(const P* str, int base)
{
    return parsesigned(str, str+stringlength(str), base);
}

template<typename T>
std::pair<uint64_t, typename T::const_iterator> parseunsigned(const T& str, int base)
{
    return parseunsigned(std::begin(str), std::end(str), base);
}

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
    V v(hexstr.size() / 2);
    size_t n = hex2binary(hexstr.begin(), hexstr.end(), v.begin(), v.end());
    v.resize(n);

    return v;
}

/*
 *   S  is a std::string, or std::string_view
 */
template<typename S>
struct stringsplitter {
    const S& _str;
    const S& _sep;

/*
    // separator, which splits according to items in, and not in the set.
    struct set {
        const S& sep;

        set(const S& sep)
            : sep(sep)
        {
        }
        auto findnext(S::const_iterator first, S::const_iterator last)
        {
        }
        auto findend(S::const_iterator first, S::const_iterator last)
        {
        }

    };
    // separator, which splits according to substrings.
    struct string {
        const S& sep;

        string(const S& sep)
            : sep(sep)
        {
        }
        auto findnext(S::const_iterator first, S::const_iterator last)
        {
        }
        auto findend(S::const_iterator first, S::const_iterator last)
        {
        }


    };
*/
    struct iter {
        typename S::const_iterator p;
        typename S::const_iterator q;
        typename S::const_iterator last;
        const S& sep;

        // TODO: change interface to 'sep' such that 'sep' decides whether to do
        // a find_first_of or a search.
        // and to treat sequences of separators as one, or as separating empty elements.
        iter(typename S::const_iterator first, typename S::const_iterator last, const S& sep)
            : p(first), q(first), last(last), sep(sep)
        {
        }
        // updates 'q', to point to the first 'sep'
        S operator*()
        {
            //q = std::find_first_of(p, last, std::begin(sep), std::end(sep));
            q = std::find_if(p, last, [this](auto c) { return std::find(std::begin(sep), std::end(sep), c) != std::end(sep); });

            //q = std::search(p, last, std::begin(sep), std::end(sep));

            return S(p, q);
        }
        // updates 'p', to point to the first 'sep'
        iter& operator++()
        {
            p = std::find_if(q, last, [this](auto c) { return std::find(std::begin(sep), std::end(sep), c) == std::end(sep); });
            //p = q + std::size(ssep)
            return *this;
        }
        bool operator!=(const iter& rhs) const
        {
            return p!=rhs.p || last!=rhs.last;
        }
    };

    stringsplitter(const S& str, const S& sep)
        : _str(str), _sep(sep)
    {
    }
    iter begin() const
    {
        return iter(std::begin(_str), std::end(_str), _sep);
    }
    iter end() const
    {
        return iter(std::end(_str), std::end(_str), _sep);
    }
};
