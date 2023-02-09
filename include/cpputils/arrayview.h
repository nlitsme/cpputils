#pragma once

#include <iomanip>
/*
 *  like string_view or span, but takes an iterator-pair.
 * 
 *  Note that array_view can also take streampos as 'P', and therefore I did not
 *  add operator[] and value_type.

 *   this is probably obsolete now that <span> is part of all standard libs.
 */
template<typename P>
class array_view {
    P first, last;
public:
    array_view()
    {
    }

    array_view(P first, P last)
        : first(first), last(last)
    {
    }
    array_view(P first, typename std::iterator_traits<P>::difference_type size)
        : first(first), last(first)
    {
        std::advance(last, size);
    }

    P begin() { return first; }
    P end() { return last; }

    P data() { return &*first; }

    auto& front() const { return *first; }
    auto& back() const { return *(last-1); }

    const P begin() const { return first; }
    const P end() const { return last; }

    size_t size() const { return last-first; }

    bool operator==( const array_view<P>& rhs ) const {
        return first==rhs.first && last==rhs.last;
    }
};


// note: for 'range' i can use either 'std::span' ( c++20 ), or std::string_view ( c++17 )
// or std::array_view (c++20)
template<typename P>
using range = array_view<P>;

template<typename P>
auto makerange(P first, P last)
{
    return range<P>(first, last);
}
template<typename P>
auto makerange(P first, size_t size)
{
    return range<P>(first, size);
}

template<typename V>
auto makerange(V & v)
{
    return range<typename V::iterator>(v.begin(), v.end());
}

template<typename V>
auto makerange(const V & v)
{
    return range<typename V::const_iterator>(v.begin(), v.end());
}

template<typename R, typename S>
bool equalrange(const R& r, const S& s)
{
    return r.size()==s.size() && std::equal(std::begin(r), std::end(r), std::begin(s));
}

/*
 * helper for printing hexdump of an array_view.
 */
template<typename P>
std::ostream& operator<<(std::ostream&os, const array_view<P>& buf)
{
    auto fillchar = os.fill();
    std::ios state(NULL);  state.copyfmt(os);

    bool first= true;
    for (const auto& b : buf) {
        os.copyfmt(state);
        if (!first && fillchar) os << fillchar;
        os << std::right;
        os << std::setfill('0');
        os << std::setw(2);
        os << std::hex << unsigned(b);
        first= false;
    }
    return os;
}
