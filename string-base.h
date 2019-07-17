#pragma once
#include <utility>
#include <iterator>
#include <algorithm>

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


