#pragma once
#include <stdint.h>

template<typename P>
struct packer_base {
    P p;
    P last;
    packer_base(P first, P last)
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
};

template<typename P>
struct unchecked_unpacker : packer_base<P> {
    unchecked_unpacker(P first, P last)
        : packer_base<P>(first, last)
    {
    }

    uint8_t get8()
    {
        return *(this->p)++;
    }
    uint16_t get16le()
    {
        uint8_t lo = get8();
        uint8_t hi = get8();
        return (uint16_t(hi)<<8) | lo;
    }
    uint32_t get32le()
    {
        uint16_t lo = get16le();
        uint16_t hi = get16le();
        return (uint32_t(hi)<<16) | lo;
    }
    uint64_t get64le()
    {
        uint32_t lo = get32le();
        uint32_t hi = get32le();
        return (uint64_t(hi)<<32) | lo;
    }
    uint16_t get16be()
    {
        uint8_t hi = get8();
        uint8_t lo = get8();
        return (uint16_t(hi)<<8) | lo;
    }
    uint32_t get32be()
    {
        uint16_t hi = get16be();
        uint16_t lo = get16be();
        return (uint32_t(hi)<<16) | lo;
    }
    uint64_t get64be()
    {
        uint32_t hi = get32be();
        uint32_t lo = get32be();
        return (uint64_t(hi)<<32) | lo;
    }
    std::string getstr(int n) { this->p += n; return std::string(this->p-n, this->p); }
    std::vector<uint8_t> getbytes(int n) { this->p += n; return std::vector<uint8_t>(this->p-n, this->p); }
};
template<typename P>
struct unpacker : unchecked_unpacker<P> {
    unpacker(P first, P last)
        : unchecked_unpacker<P>(first, last)
    {
    }

    uint8_t get8() { this->require(1); return unchecked_unpacker<P>::get8(); }
    uint16_t get16le() { this->require(2); return unchecked_unpacker<P>::get16le(); }
    uint32_t get32le() { this->require(4); return unchecked_unpacker<P>::get32le(); }
    uint64_t get64le() { this->require(8); return unchecked_unpacker<P>::get64le(); }
    uint16_t get16be() { this->require(2); return unchecked_unpacker<P>::get16be(); }
    uint32_t get32be() { this->require(4); return unchecked_unpacker<P>::get32be(); }
    uint64_t get64be() { this->require(8); return unchecked_unpacker<P>::get64be(); }

    std::string getstr(int n) { this->require(n); return unchecked_unpacker<P>::getstr(n); }
    std::vector<uint8_t> getbytes(int n) { this->require(n); return unchecked_unpacker<P>::getbytes(n); }
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
struct unchecked_packer : packer_base<P> {
    unchecked_packer(P first, P last)
        : packer_base<P>(first, last)
    {
    }

    void set8(uint8_t value)
    {
        *(this->p)++ = value;
    }
    void set16le(uint16_t value)
    {
        set8(value);
        set8(value>>8);
    }
    void set32le(uint32_t value)
    {
        set16le(value);
        set16le(value>>16);
    }
    void set64le(uint64_t value)
    {
        set32le(value);
        set32le(value>>32);
    }
    void set16be(uint16_t value)
    {
        set8(value>>8);
        set8(value);
    }
    void set32be(uint32_t value)
    {
        set16be(value>>16);
        set16be(value);
    }
    void set64be(uint64_t value)
    {
        set32be(value>>32);
        set32be(value);
    }
    void setstr(const std::string& txt) { std::copy(txt.begin(), txt.end(), this->p); this->p += txt.size(); }
    void setbytes(const std::vector<uint8_t>& data) { std::copy(data.begin(), data.end(), this->p); this->p += data.size(); }
};
template<typename P>
struct packer : unchecked_packer<P> {
    packer(P first, P last)
        : unchecked_packer<P>(first, last)
    {
    }

    void set8(uint8_t value) { this->require(1); unchecked_packer<P>::set8(value); }
    void set16le(uint16_t value) { this->require(2); unchecked_packer<P>::set16le(value); }
    void set32le(uint32_t value) { this->require(4); unchecked_packer<P>::set32le(value); }
    void set64le(uint64_t value) { this->require(8); unchecked_packer<P>::set64le(value); }
    void set16be(uint16_t value) { this->require(2); unchecked_packer<P>::set16be(value); }
    void set32be(uint32_t value) { this->require(4); unchecked_packer<P>::set32be(value); }
    void set64be(uint64_t value) { this->require(8); unchecked_packer<P>::set64be(value); }

    void setstr(const std::string& txt) { this->require(txt.size()); unchecked_packer<P>::setstr(txt); }
    void setbytes(const std::vector<uint8_t>& data) { this->require(data.size()); unchecked_packer<P>::setbytes(data); }
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
    static uint8_t get8(P p) { return unchecked_unpacker<P>(p, p).get8(); }
template<typename P>
    static uint16_t get16le(P p) { return unchecked_unpacker<P>(p, p).get16le(); }
template<typename P>                                                      
    static uint32_t get32le(P p) { return unchecked_unpacker<P>(p, p).get32le(); }
template<typename P>                                                      
    static uint64_t get64le(P p) { return unchecked_unpacker<P>(p, p).get64le(); }
template<typename P>                                                      
    static uint16_t get16be(P p) { return unchecked_unpacker<P>(p, p).get16be(); }
template<typename P>                                                      
    static uint32_t get32be(P p) { return unchecked_unpacker<P>(p, p).get32be(); }
template<typename P>                                                      
    static uint64_t get64be(P p) { return unchecked_unpacker<P>(p, p).get64be(); }
template<typename P>                                                      
    static std::string getstr(P p, size_t n) { return unchecked_unpacker<P>(p, p).getstr(n); }
template<typename P>                                                      
    static std::vector<uint8_t> getbytes(P p, size_t n) { return unchecked_unpacker<P>(p, p).getbytes(n); }
template<typename P>
    static void set8(P p, uint8_t x) { unchecked_packer<P>(p, p).set8(x); }
template<typename P>
    static void set16le(P p, uint16_t x) { unchecked_packer<P>(p, p).set16le(x); }
template<typename P>
    static void set32le(P p, uint32_t x) { unchecked_packer<P>(p, p).set32le(x); }
template<typename P>
    static void set64le(P p, uint64_t x) { unchecked_packer<P>(p, p).set64le(x); }
template<typename P>
    static void set16be(P p, uint16_t x) { unchecked_packer<P>(p, p).set16be(x); }
template<typename P>
    static void set32be(P p, uint32_t x) { unchecked_packer<P>(p, p).set32be(x); }
template<typename P>
    static void set64be(P p, uint64_t x) { unchecked_packer<P>(p, p).set64be(x); }
template<typename P>                                                      
    static void setstr(P p, const std::string& txt) { return unchecked_unpacker<P>(p, p).setstr(txt); }
template<typename P>                                                      
    static void setbytes(P p, const std::vector<uint8_t>& data) { return unchecked_unpacker<P>(p, p).setbytes(data); }

};
