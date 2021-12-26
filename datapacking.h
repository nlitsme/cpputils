#pragma once
#include <stdint.h>
#include <vector>
#include <array>
#include <string>
#include <stdexcept>
#include <algorithm>

#include "templateutils.h"

// todo: add 'P copybytes([int n, ]range-object)' which copies (n or range.size()) bytes
// to range, and returns the ptr to the next item to be written.

template<typename P>
struct packer_base {
    P p;
    P last;
    packer_base(P first, P last)
        : p(first), last(last)
    {
    }

    bool eof() const
    {
        return p == last;
    }

    void require(int n)
    {
        if (!have(n))
            throw std::runtime_error("not enough data");
    }
    bool have(int n)
    {
        if constexpr (!std::is_void_v<typename std::iterator_traits<P>::difference_type>)
            return n <= last-p;

        // with output iterator i can't check if there is enough space,
        // usually this is the case with the back_insert_iterator.
        return true;
    }
    void skip(int n)
    {
        require(n);
        p += n;
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
    uint32_t get24le()
    {
        uint16_t lo = get16le();
        uint8_t hi = get8();
        return (uint32_t(hi)<<16) | lo;
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
    uint32_t get24be()
    {
        uint8_t hi = get8();
        uint16_t lo = get16be();
        return (uint32_t(hi)<<8) | lo;
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
    std::string getzstr() { 
        auto z = std::find(unchecked_unpacker<P>::p, unchecked_unpacker<P>::last, 0);
        auto str = unchecked_unpacker<P>::getstr(z-unchecked_unpacker<P>::p);
        unchecked_unpacker<P>::skip(1);
        return str;
    }

    std::vector<uint8_t> getbytes(int n) { this->p += n; return std::vector<uint8_t>(this->p-n, this->p); }

    const uint8_t *getdata(int n) { this->p += n; return &*(this->p-n); }
};
template<typename P>
struct unpacker : unchecked_unpacker<P> {
    unpacker(P first, P last)
        : unchecked_unpacker<P>(first, last)
    {
    }

    uint8_t get8() { this->require(1); return unchecked_unpacker<P>::get8(); }
    uint16_t get16le() { this->require(2); return unchecked_unpacker<P>::get16le(); }
    uint32_t get24le() { this->require(3); return unchecked_unpacker<P>::get24le(); }
    uint32_t get32le() { this->require(4); return unchecked_unpacker<P>::get32le(); }
    uint64_t get64le() { this->require(8); return unchecked_unpacker<P>::get64le(); }
    uint16_t get16be() { this->require(2); return unchecked_unpacker<P>::get16be(); }
    uint32_t get24be() { this->require(3); return unchecked_unpacker<P>::get24be(); }
    uint32_t get32be() { this->require(4); return unchecked_unpacker<P>::get32be(); }
    uint64_t get64be() { this->require(8); return unchecked_unpacker<P>::get64be(); }

    std::string getstr(int n) { this->require(n); return unchecked_unpacker<P>::getstr(n); }
    std::string getzstr() { 
        auto z = std::find(unchecked_unpacker<P>::p, unchecked_unpacker<P>::last, 0);
        if (z==unchecked_unpacker<P>::last)
            throw std::runtime_error("missing nul after ztring");
        auto str = unchecked_unpacker<P>::getstr(z-unchecked_unpacker<P>::p);
        unchecked_unpacker<P>::skip(1);
        return str;
    }
    std::vector<uint8_t> getbytes(int n) { this->require(n); return unchecked_unpacker<P>::getbytes(n); }

    const uint8_t *getdata(int n) { this->require(n); return unchecked_unpacker<P>::getdata(n); }
};
template<typename CONTAINER, typename dummy = std::enable_if_t<is_container_v<CONTAINER> > >
auto makeunpacker(CONTAINER& v)
{
    return unpacker(v.begin(), v.end());
}
template<typename P>
auto makeunpacker(P first, P last)
{
    return unpacker(first, last);
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
    void set24le(uint32_t value)
    {
        set16le(value);
        set8(value>>16);
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
    void set24be(uint32_t value)
    {
        set8(value>>16);
        set16be(value);
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
    void setzstr(const std::string& txt) { setstr(txt); set8(0); }

    template<typename CONTAINER, typename dummy = std::enable_if_t<is_container_v<CONTAINER> > >
    void setbytes(const CONTAINER& data) { std::copy(data.begin(), data.end(), this->p); this->p += data.size(); }
    template<typename Q>
    void setbytes(Q first, Q last) { std::copy(first, last, this->p); this->p += std::distance(first, last); }
};
template<typename P>
struct packer : unchecked_packer<P> {
    packer(P first, P last)
        : unchecked_packer<P>(first, last)
    {
    }

    void set8(uint8_t value) { this->require(1); unchecked_packer<P>::set8(value); }
    void set16le(uint16_t value) { this->require(2); unchecked_packer<P>::set16le(value); }
    void set24le(uint32_t value) { this->require(3); unchecked_packer<P>::set24le(value); }
    void set32le(uint32_t value) { this->require(4); unchecked_packer<P>::set32le(value); }
    void set64le(uint64_t value) { this->require(8); unchecked_packer<P>::set64le(value); }
    void set16be(uint16_t value) { this->require(2); unchecked_packer<P>::set16be(value); }
    void set24be(uint32_t value) { this->require(3); unchecked_packer<P>::set24be(value); }
    void set32be(uint32_t value) { this->require(4); unchecked_packer<P>::set32be(value); }
    void set64be(uint64_t value) { this->require(8); unchecked_packer<P>::set64be(value); }

    void setstr(const std::string& txt) { this->require(txt.size()); unchecked_packer<P>::setstr(txt); }
    void setzstr(const std::string& txt) { setstr(txt); set8(0); }

    template<typename CONTAINER, typename dummy = std::enable_if_t<is_container_v<CONTAINER> > >
    void setbytes(const CONTAINER& data) { this->require(data.size()); unchecked_packer<P>::setbytes(data); }
    template<typename Q>
    void setbytes(Q first, Q last) { this->require(std::distance(first, last)); unchecked_packer<P>::setbytes(first, last); }

};
template<typename CONTAINER, typename dummy = std::enable_if_t<is_container_v<CONTAINER> > >
auto makepacker(CONTAINER& v)
{
    return packer(v.begin(), v.end());
}
template<typename P>
auto makepacker(P first, P last)
{
    return packer(first, last);
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
    static uint32_t get24le(P p) { return unchecked_unpacker<P>(p, p).get24le(); }
template<typename P>                                                      
    static uint32_t get32le(P p) { return unchecked_unpacker<P>(p, p).get32le(); }
template<typename P>                                                      
    static uint64_t get64le(P p) { return unchecked_unpacker<P>(p, p).get64le(); }
template<typename P>                                                      
    static uint16_t get16be(P p) { return unchecked_unpacker<P>(p, p).get16be(); }
template<typename P>                                                      
    static uint32_t get24be(P p) { return unchecked_unpacker<P>(p, p).get24be(); }
template<typename P>                                                      
    static uint32_t get32be(P p) { return unchecked_unpacker<P>(p, p).get32be(); }
template<typename P>                                                      
    static uint64_t get64be(P p) { return unchecked_unpacker<P>(p, p).get64be(); }
template<typename P>                                                      
    static std::string getstr(P p, size_t n) { return unchecked_unpacker<P>(p, p).getstr(n); }
template<typename P>                                                      
    static std::string getzstr(P p) { return unchecked_unpacker<P>(p, p).getzstr(); }
template<typename P>                                                      
    static std::vector<uint8_t> getbytes(P p, size_t n) { return unchecked_unpacker<P>(p, p).getbytes(n); }
template<typename P>
    static void set8(P p, uint8_t x) { unchecked_packer<P>(p, p).set8(x); }
template<typename P>
    static void set16le(P p, uint16_t x) { unchecked_packer<P>(p, p).set16le(x); }
template<typename P>
    static void set24le(P p, uint32_t x) { unchecked_packer<P>(p, p).set24le(x); }
template<typename P>
    static void set32le(P p, uint32_t x) { unchecked_packer<P>(p, p).set32le(x); }
template<typename P>
    static void set64le(P p, uint64_t x) { unchecked_packer<P>(p, p).set64le(x); }
template<typename P>
    static void set16be(P p, uint16_t x) { unchecked_packer<P>(p, p).set16be(x); }
template<typename P>
    static void set24be(P p, uint32_t x) { unchecked_packer<P>(p, p).set24be(x); }
template<typename P>
    static void set32be(P p, uint32_t x) { unchecked_packer<P>(p, p).set32be(x); }
template<typename P>
    static void set64be(P p, uint64_t x) { unchecked_packer<P>(p, p).set64be(x); }
template<typename P>                                                      
    static void setstr(P p, const std::string& txt) { unchecked_packer<P>(p, p).setstr(txt); }
template<typename P>                                                      
    static void setzstr(P p, const std::string& txt) { unchecked_packer<P>(p, p).setzstr(txt); }
template<typename P, typename CONTAINER, typename dummy = std::enable_if_t<is_container_v<CONTAINER> > >
    static void setbytes(P p, const CONTAINER& data) { unchecked_packer<P>(p, p).setbytes(data); }

};
