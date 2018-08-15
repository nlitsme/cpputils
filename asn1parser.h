#pragma once
#include <memory>   // shared_ptr

#include "arrayview.h"

/*
 * decodes one t,l,v triplet from a BER asn1 sequence.
 */
template<typename P>
struct asn1tlv {
    bool constructed = false;
    int cls = 0;
    unsigned int tagvalue = 0;
    int length = 0;

    range<P>  tagrange;
    range<P>  lenrange;
    range<P>  datarange;

    asn1tlv(range<P> r)
        : asn1tlv(std::begin(r), std::end(r))
    {
    }
    asn1tlv(P first, P last)
    {
        P p = first;
        if (last-p<2)
            throw "not enough data";

        auto tag_start = p;

        uint8_t hbyte = *p++;
        constructed = (hbyte&0x20) != 0;
        cls = hbyte>>6;
        tagvalue = hbyte&0x1F;
        if (tagvalue==0x1F) {
            auto [tv, q] = readvariable(p, last, 32);

            tagvalue = tv;
            p = q;
        }

        auto tag_end = p;

        tagrange = makerange(tag_start, tag_end);

        if (p==last)
            throw "not enough data";

        auto len_start = p;
        auto len_end = p;
        uint8_t lbyte = *p++;
        if (lbyte < 0x80) {
            len_end = p;
            length = lbyte;
        }
        else if (lbyte == 0x80) {
            len_end = p;
            length = -1;
        }
        else {
            len_end = p+(lbyte&0x7F);
            if (len_end>=last)
                throw "not enough data";
            length = readfixed(p, len_end);
            p = len_end;
        }
        lenrange = makerange(len_start, len_end);

        datarange = makerange(p, p+length);
    }
    auto readvariable(P first, P last, int bitlimit)
    {
        int value = 0;
        bool done = false;
        int bitcount = 0;
        P p = first;
        while (!done && p<last && bitcount < bitlimit) {
            uint8_t byte = *p++;
            value <<= 7;
            value |= byte&0x7F;
            done = (byte&0x80)==0;

            bitcount += 7;
        }

        if (!done)
            throw "not enough data";
        return std::make_tuple(value, p);
    }
    auto readfixed(P first, P last)
    {
        uint64_t value = 0;
        auto p = first;

        while (p<last)
        {
            value <<= 8;
            value |= *p++;
        }
        return value;
    }
};


/*
 *  reads an asn1 object from a stream, without reading the actual data bytes.
 */
template<>
struct asn1tlv<std::istream::pos_type> {
    bool constructed = false;
    int cls = 0;
    unsigned int tagvalue = 0;
    int length = 0;

    range<std::istream::pos_type>  tagrange;
    range<std::istream::pos_type>  lenrange;
    range<std::istream::pos_type>  datarange;


    auto readvariable(std::istream& is, int bitlimit)
    {
        int value = 0;
        bool done = false;
        int bitcount = 0;

        while (!done && bitcount < bitlimit) {
            uint8_t byte = is.get();
            if (byte<0)
                throw "truncated integer";
            value <<= 7;
            value |= byte&0x7F;
            done = (byte&0x80)==0;

            bitcount += 7;
        }

        if (!done)
            throw "not enough data";
        return value;
    }
    auto readfixed(std::istream& is, int n)
    {
        uint64_t value = 0;

        while (!is.eof() && n-- > 0)
        {
            auto byte = is.get();
            if (byte<0)
                throw "truncated integer";
            value <<= 8;
            value |= byte;
        }
        return value;
    }

    asn1tlv(std::istream& is)
    {
        auto tag_start = is.tellg();

        auto hbyte = is.get();
        if (hbyte < 0)
            throw "truncated header";

        constructed = (hbyte&0x20) != 0;
        cls = hbyte>>6;
        tagvalue = hbyte&0x1F;
        if (tagvalue==0x1F) {
            auto tv = readvariable(is, 32);

            tagvalue = tv;
        }

        auto tag_end = is.tellg();

        tagrange = makerange(tag_start, tag_end);

        auto len_start = is.tellg();
        auto len_end = is.tellg();
        auto lbyte = is.get();
        if (lbyte < 0)
            throw "truncated length";

        if (lbyte < 0x80) {
            len_end = is.tellg();
            length = lbyte;
        }
        else if (lbyte == 0x80) {
            len_end = is.tellg();
            length = -1;
        }
        else {
            len_end = is.tellg()+std::streamoff(lbyte&0x7F);
            length = readfixed(is, len_end-len_start-1);
        }
        lenrange = makerange(len_start, len_end);

        datarange = makerange(is.tellg(), is.tellg()+std::streamoff(length));
    }
};

/*
 * class for enumerating concatenated tlv objects
 *
 * for (auto & tlv : enumtlvs(range)) {
 * }
 */

template<typename P>
class enumtlvs {
    class tlviterator
    {
    public:
        typedef std::forward_iterator_tag iterator_category;
        typedef asn1tlv<P> value_type;
        typedef int difference_type;
        typedef value_type* pointer;
        typedef value_type& reference;

    private:
        std::shared_ptr<value_type> item;

        P cur;
        P last;
    public:
        tlviterator(P first, P last) : cur(first), last(last) { }

        tlviterator& operator++() { 
            cur = (*(*this)).datarange.end();

            if (cur < last)
                item = std::make_shared<value_type>(cur, last);
            else
                item.reset();
            return *this;
        }
        tlviterator operator++(int) { tlviterator val = *this; ++(*this); return val; }

        bool operator==(const tlviterator& rhs) const { return cur==rhs.cur; }
        bool operator!=(const tlviterator& rhs) const { return !(*this==rhs); }

        value_type operator*() {
            if (!item)
                item = std::make_shared<value_type>(cur, last);
            return *item.get();
        }
    };

    P first;
    P last;
public:

    enumtlvs(range<P> range)
        : first(std::begin(range)), last(std::end(range))
    {
    }

    tlviterator begin() const { return tlviterator(first, last); }
    tlviterator end() const { return tlviterator(last, last); }
};

/*
 * get one path item from the 'asn1-tree'
 */
template<typename ASNOBJ>
ASNOBJ get_nth_tlv(ASNOBJ obj, int n)
{
    int i = 0;
    for (auto tlv : enumtlvs(obj.datarange)) {
        if (tlv.cls==0) {
            if (i==n)
                return tlv;
            i++;
        }
        else {
            if (tlv.tagvalue == -n-1)
                return tlv;
        }
    }
    throw "missing item";
}

/*
 *  Get asn1-subtree by path.
 *
 *  An asn1 object can be viewed as a tree like object.
 *  with optional 'CONTEXT' objects specified by negative index values.
 */
template<typename ASNOBJ>
ASNOBJ traverse(ASNOBJ obj, std::vector<int> path)
{
    for (int ix : path)
        obj = get_nth_tlv(obj, ix);
    return obj;
}



