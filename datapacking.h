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
        return p+n <= last;
    }
    uint8_t _get8()
    {
        return *p++;
    }
    uint16_t _get16le()
    {
        uint8_t lo = get8();
        uint8_t hi = get8();
        return (uint16_t(hi)<<8) | lo;
    }
    uint32_t _get32le()
    {
        uint16_t lo = get16le();
        uint16_t hi = get16le();
        return (uint32_t(hi)<<16) | lo;
    }
    uint64_t _get64le()
    {
        uint32_t lo = get32le();
        uint32_t hi = get32le();
        return (uint64_t(hi)<<32) | lo;
    }
    uint16_t _get16be()
    {
        uint8_t hi = get8();
        uint8_t lo = get8();
        return (uint16_t(hi)<<8) | lo;
    }
    uint32_t _get32be()
    {
        uint16_t hi = get16be();
        uint16_t lo = get16be();
        return (uint32_t(hi)<<16) | lo;
    }
    uint64_t _get64be()
    {
        uint32_t hi = get32be();
        uint32_t lo = get32be();
        return (uint64_t(hi)<<32) | lo;
    }
    uint8_t get8() { require(1); return _get8(); }
    uint16_t get16le() { require(2); return _get16le(); }
    uint32_t get32le() { require(4); return _get32le(); }
    uint64_t get64le() { require(8); return _get64le(); }
    uint16_t get16be() { require(2); return _get16be(); }
    uint32_t get32be() { require(4); return _get32be(); }
    uint64_t get64be() { require(8); return _get64be(); }
    std::string getstr(int n) { require(n); p += n; return std::string((const char*)p-n, p); }
    std::vector<uint8_t> getbytes(int n) { require(n); p += n; return std::vector<uint8_t>((const uint8_t*)p-n, p); }

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
