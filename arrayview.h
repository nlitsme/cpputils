#pragma once

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
    P begin() { return first; }
    P end() { return last; }

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
        os.fill('0');
        os.width(2);
        os << std::hex << unsigned(b);
        first= false;
    }
    return os;
}
