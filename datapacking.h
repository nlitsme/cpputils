#pragma once
#include <stdint.h>

template<typename P>
class unpacker {
    P p;
    P last;
public:
    unpacker(P first, P last)
        : p(first), last(last)
    {
    }

    void require(int n)
    {
        if (!have(n))
            throw "not enough data";
    }
    bool have(int n)
    {
        if constexpr (!std::is_void_v<typename std::iterator_traits<P>::difference_type>)
            return p+n <= last;

        // with output iterator i can't check if there is enough space,
        // usually this is the case with the back_insert_iterator.
        return true;
    }
    uint8_t _get8()
    {
        return *p++;
    }
    uint16_t _get16le()
    {
        uint8_t lo = _get8();
        uint8_t hi = _get8();
        return (uint16_t(hi)<<8) | lo;
    }
    uint32_t _get32le()
    {
        uint16_t lo = _get16le();
        uint16_t hi = _get16le();
        return (uint32_t(hi)<<16) | lo;
    }
    uint64_t _get64le()
    {
        uint32_t lo = _get32le();
        uint32_t hi = _get32le();
        return (uint64_t(hi)<<32) | lo;
    }
    uint16_t _get16be()
    {
        uint8_t hi = _get8();
        uint8_t lo = _get8();
        return (uint16_t(hi)<<8) | lo;
    }
    uint32_t _get32be()
    {
        uint16_t hi = _get16be();
        uint16_t lo = _get16be();
        return (uint32_t(hi)<<16) | lo;
    }
    uint64_t _get64be()
    {
        uint32_t hi = _get32be();
        uint32_t lo = _get32be();
        return (uint64_t(hi)<<32) | lo;
    }

    uint8_t get8() { require(1); return _get8(); }
    uint16_t get16le() { require(2); return _get16le(); }
    uint32_t get32le() { require(4); return _get32le(); }
    uint64_t get64le() { require(8); return _get64le(); }
    uint16_t get16be() { require(2); return _get16be(); }
    uint32_t get32be() { require(4); return _get32be(); }
    uint64_t get64be() { require(8); return _get64be(); }

    std::string getstr(int n) { require(n); p += n; return std::string(p-n, p); }
    std::vector<uint8_t> getbytes(int n) { require(n); p += n; return std::vector<uint8_t>(p-n, p); }
};
template<typename T, typename A>
auto makeunpacker(std::vector<T, A>& v)
{
    return unpacker(v.begin(), v.end());
}
template<typename T, int N>
auto makeunpacker(std::array<T, N>& v)
{
    return unpacker(v.begin(), v.end());
}
template<typename T, typename A>
auto makeunpacker(std::basic_string<T, A>& v)
{
    return unpacker(v.begin(), v.end());
}
template<typename P>
class packer {
    P p;
    P last;
public:
    packer(P first, P last)
        : p(first), last(last)
    {
    }

    void require(int n)
    {
        if (!have(n))
            throw "not enough data";
    }
    bool have(int n)
    {
        if constexpr (!std::is_void_v<typename std::iterator_traits<P>::difference_type>)
            return p+n <= last;

        // with output iterator i can't check if there is enough space,
        // usually this is the case with the back_insert_iterator.
        return true;
    }
    void _set8(uint8_t value)
    {
        *p++ = value;
    }
    void _set16le(uint16_t value)
    {
        _set8(value);
        _set8(value>>8);
    }
    void _set32le(uint32_t value)
    {
        _set16le(value);
        _set16le(value>>16);
    }
    void _set64le(uint64_t value)
    {
        _set32le(value);
        _set32le(value>>32);
    }
    void _set16be(uint16_t value)
    {
        _set8(value>>8);
        _set8(value);
    }
    void _set32be(uint32_t value)
    {
        _set16be(value>>16);
        _set16be(value);
    }
    void _set64be(uint64_t value)
    {
        _set32be(value>>32);
        _set32be(value);
    }
    void set8(uint8_t value) { require(1); _set8(value); }
    void set16le(uint16_t value) { require(2); _set16le(value); }
    void set32le(uint32_t value) { require(4); _set32le(value); }
    void set64le(uint64_t value) { require(8); _set64le(value); }
    void set16be(uint16_t value) { require(2); _set16be(value); }
    void set32be(uint32_t value) { require(4); _set32be(value); }
    void set64be(uint64_t value) { require(8); _set64be(value); }

    void setstr(const std::string& txt) { require(txt.size()); std::copy(txt.begin(), txt.end(), p); p += txt.size(); }
    void setbytes(const std::vector<uint8_t>& data) { require(data.size()); std::copy(data.begin(), data.end(), p); p += data.size(); }
};
template<typename T, typename A>
auto makepacker(std::vector<T, A>& v)
{
    return packer(v.begin(), v.end());
}
template<typename T, int N>
auto makepacker(std::array<T, N>& v)
{
    return packer(v.begin(), v.end());
}
template<typename T, typename A>
auto makepacker(std::basic_string<T, A>& v)
{
    return packer(v.begin(), v.end());
}

/*
 *  unchecked packer
 */
struct unchecked {
template<typename P>
    static uint8_t get8(P p) { return *p; }
template<typename P>
    static uint16_t get16le(P p)
    {
        uint8_t lo = get8(p);
        uint8_t hi = get8(p+1);
        return (uint16_t(hi)<<8) | lo;
    }
template<typename P>
    static uint32_t get32le(P p)
    {
        uint16_t lo = get16le(p);
        uint16_t hi = get16le(p+2);
        return (uint32_t(hi)<<16) | lo;
    }
template<typename P>
    static uint64_t get64le(P p)
    {
        uint32_t lo = get32le(p);
        uint32_t hi = get32le(p+4);
        return (uint64_t(hi)<<32) | lo;
    }
template<typename P>
    static uint16_t get16be(P p)
    {
        uint8_t hi = get8(p);
        uint8_t lo = get8(p+1);
        return (uint16_t(hi)<<8) | lo;
    }
template<typename P>
    static uint32_t get32be(P p)
    {
        uint16_t hi = get16be(p);
        uint16_t lo = get16be(p+2);
        return (uint32_t(hi)<<16) | lo;
    }
template<typename P>
    static uint64_t get64be(P p)
    {
        uint32_t hi = get32be(p);
        uint32_t lo = get32be(p+4);
        return (uint64_t(hi)<<32) | lo;
    }
template<typename P>
    static void set8(P p, uint8_t x) { *p = x; }
template<typename P>
    static void set16le(P p, uint16_t x)
    {
        set8(p, x);
        set8(p+1, x>>8);
    }
template<typename P>
    static void set32le(P p, uint32_t x)
    {
        set16le(p, x);
        set16le(p+2, x>>16);
    }
template<typename P>
    static void set64le(P p, uint64_t x)
    {
        set32le(p, x);
        set32le(p+4, x>>32);
    }
template<typename P>
    static void set16be(P p, uint16_t x)
    {
        set8(p, x>>8);
        set8(p+1, x);
    }
template<typename P>
    static void set32be(P p, uint32_t x)
    {
        set16be(p, x>>16);
        set16be(p+2, x);
    }
template<typename P>
    static void set64be(P p, uint64_t x)
    {
        set32be(p, x>>32);
        set32be(p+4, x);
    }

};
