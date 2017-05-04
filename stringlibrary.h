#pragma once

#include <utility>

// todo: strip, lstrip, rstrip
//       split, join


template<typename T>
size_t stringlength(const T *str)
{
    size_t len=0;
    while (*str++)
        len++;
    return len;
}

template<typename T>
int stringcompare(const T *a, const T *b)
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

template<typename T>
T *stringcopy(T *a, const T *b)
{
    while ((*a++ = *b++)!=0)
        ;
    return a-1;
}

template<typename T>
int charicompare(T a,T b)
{
    a=(T)tolower(a);
    b=(T)tolower(b);
    if (a<b) return -1;
    if (a>b) return 1;
    return 0;
}

template<class P>
int stringicompare(const P* a, const P* b)
{
    while (*a && *b && charicompare(*a, *b)==0)
    {
        a++;
        b++;
    }
    return charicompare(*a, *b);
}

template<class T>
int stringicompare(const T& a, const T& b)
{
    typename T::const_iterator pa= a.begin();
    typename T::const_iterator pa_end= a.end();
    typename T::const_iterator pb= b.begin();
    typename T::const_iterator pb_end= b.end();
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
                    state++;
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
                else if (0<=n && n<=7) {
                    base = 8;
                    state = 3;
                    num = n;  // this is the first real digit
                    digits = 1;
                }
                else if (n>=8) {
                    // invalid
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


