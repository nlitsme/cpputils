#pragma once
#include <memory>   // shared_ptr
#include <iostream>
#include <vector>

#include "arrayview.h"

enum {
    CLS_UNIVERSAL,
    CLS_APPLICATION,
    CLS_CONTEXT,
    CLS_PRIVATE
};
enum {  // universal
    TAG_EOC,                  //  0
    TAG_BOOLEAN,              //  1
    TAG_INTEGER,              //  2
    TAG_BITSTRING,            //  3
    TAG_OCTETSTRING,          //  4
    TAG_NULL,                 //  5
    TAG_OID,                  //  6
    TAG_OBJDESC,              //  7
    TAG_EXTTYPE,              //  8
    TAG_REAL,                 //  9
    TAG_ENUM,                 // 10
    TAG_EMBEDDED,             // 11
    TAG_UTF8STRING,           // 12
    TAG_RELATIVEOID,          // 13
    TAG_TIME,                 // 14
    TAG_0f,                   // 15
    TAG_SEQUENCE,             // 16
    TAG_SET,                  // 17
    TAG_NUMERICSTRING,        // 18
    TAG_PRINTABLESTRING,      // 19
    TAG_T61STRING,            // 20
    TAG_VIDEOTEXSTRING,       // 21
    TAG_IASSTRING,            // 22
    TAG_TIMESTAMP,            // 23  UTCTIme
    TAG_FULLSTAMP,            // 24  GeneralizedTime
    TAG_GRAPHICSTRING,        // 25
    TAG_VISIBLESTRING,        // 26
    TAG_GENERALSTRING,        // 27
    TAG_UNIVERSALSTRING,      // 28
    TAG_UNRESTRICTEDSTRING,   // 29
    TAG_BMPSTRING,            // 30
    TAG_DATE,                 // 31
    TAG_TIMEOFDAY,            // 32
    TAG_DATETIME,             // 33
    TAG_DURATION,             // 34
    TAG_RIDTYPE,              // 35  OID internationalized resource identifier type
    TAG_RRIDTYPE,             // 36  Relative OID internationalized resource identifier type
};
/*
 * decodes one t,l,v triplet from a BER asn1 sequence.
 *
 * see asn1-ber-X.690-0207.pdf
 *
 * properties:
 *    constructed
 *    cls
 *    tagvalue   or tagrange
 *    length     or lenrange
 *
 *    datarange
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
            throw std::runtime_error("not enough data");

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
            throw std::runtime_error("not enough data");

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
            lbyte &= 0x7F;
            if (last-p < lbyte)
                throw std::runtime_error("not enough data");

            len_end = p+lbyte;
            if (len_end>last)
                throw std::runtime_error("not enough data");
            length = readfixed(p, len_end);
            p = len_end;
        }
        lenrange = makerange(len_start, len_end);

        if (last-p < length)
            throw std::runtime_error("not enough data");

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
            throw std::runtime_error("not enough data");
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

template<typename P>
auto gettlv(P first, P last)
{
    return asn1tlv<P>(first, last);
}

namespace std {
template<>
struct iterator_traits<std::istream::pos_type>
{
    typedef std::istream::off_type difference_type;
};
}

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
                throw std::runtime_error("truncated integer");
            value <<= 7;
            value |= byte&0x7F;
            done = (byte&0x80)==0;

            bitcount += 7;
        }

        if (!done)
            throw std::runtime_error("not enough data");
        return value;
    }
    auto readfixed(std::istream& is, int n)
    {
        uint64_t value = 0;

        while (!is.eof() && n-- > 0)
        {
            auto byte = is.get();
            if (byte<0)
                throw std::runtime_error("truncated integer");
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
            throw std::runtime_error("truncated header");

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
            throw std::runtime_error("truncated length");

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
    enumtlvs(P first, P last)
        : first(first), last(last)
    {
    }

    tlviterator begin() const { return tlviterator(first, last); }
    tlviterator end() const { return tlviterator(last, last); }
};

/*
 * get one path item from the 'asn1-tree'
 *
 *    n >= 0  --> take the n-th class0 item
 *    n < 0   --> take the item where tagvalue == -(n+1)
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
    throw std::runtime_error("missing item");
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



