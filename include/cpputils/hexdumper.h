#pragma once
/*
 * A hexdumper using iostream manipulators
 *
 * (C) 2016 Willem Hengeveld <itsme@xs4all.nl>
 */
#include <ostream>
#include <iomanip>

//
// ... design ...
//
// how to use the stream to configure the hexdump?
//
// it is possible to add custom io manipulators
//    they have to confirm to this prototype:
//       ostream& custom(ostream& os)
//    there are at least 16 bits of fmt flags left.
//
// hex:ascdump  ... but i could use std::left ('-'), std::showpos ('+') for that.
//
//
// you can set arbitrary values in a stream using
//     __i = os.xalloc(); ... obtain index to be used
//     os.iword(__i) = value
//     ... then use os.iword(__i)
//
//
// adjustfield
//     '-' left =  only hexdump 
//     '+' showpos =  only ascdump
//     internal  = ??
// basefield
//     hex/dec/oct  = numeric base representation
// showbase, uppercase
// width    = nr of units per line.  -1 -> autoformat
// precision = nr of bytes per unit
// showpoint       -> 'show offset' :  iword(__baseofs)  is valid.
// skipws          -> summarize identical lines
// noskipws        -> don't summarize
//
// unused stream flags:
//
// boolalpha     bool is output as "true", instead of "1"
// floatfield = fixed | scientific
// showpos       ... show '+' and '-'
// unitbuf       flush after every output


// special stream manipulators
//                      uint8_t {1,2,97,98,5}
//
//  hexstring        -- "0102616205"
//  ascstring        -- "..ab."
//  multiline        -- <16 bytes> \n <16 bytes> \n
//  singleline       -- 01 02 61 62 05
//  bin              -- 00000001 00000010 ...
//  
//  offset(o)        -- 00000000: 01 02 61 62 05
//  
//  step(s)          -- 00000000: .... \n 00001000: .... \n ...
//  
//  
// TODO:
//   * support dumping streams
//   * support dumping container types, ranges, iterators.
//  

namespace Hex {

struct Hexdumper_base  {
    static int __baseofs() { static int value = std::ostream::xalloc(); return value; }
    static int __step() { static int value = std::ostream::xalloc(); return value; }
    static int __threshold() { static int value = std::ostream::xalloc(); return value; }
    static int __flags() { static int value = std::ostream::xalloc(); return value; }
    enum { AS_BINARY = 1 };

    static uint64_t getbaseofs(std::ostream& os) { return os.iword(__baseofs()); }
    static void setbaseofs(std::ostream& os, uint64_t ofs) { os.iword(__baseofs()) = ofs; }

    static uint64_t getstep(std::ostream& os) { return os.iword(__step()); }
    static void setstep(std::ostream& os, uint64_t ofs) { os.iword(__step()) = ofs; }

    static uint64_t getthreshold(std::ostream& os) { return os.iword(__threshold()); }
    static void setthreshold(std::ostream& os, uint64_t ofs) { os.iword(__threshold()) = ofs; }

    static bool getbin(std::ostream& os) { return os.iword(__flags()) & AS_BINARY; }
    static void setbin(std::ostream& os) { os.iword(__flags()) |= AS_BINARY; }
    static void clearflags(std::ostream& os) {
        os.iword(__flags()) = 0;
        os.iword(__step()) = 0;
        os.iword(__threshold()) = 2;
    }
};

template<typename T>
class Hexdumper : public Hexdumper_base {
    const T * _first;
    const T * _last;

    public:

    Hexdumper(const T* first, const T* last)
    {
        _first = first;
        _last = last;
    }
    template<typename PAIR>
    static bool data_is_equal(const PAIR& a, const PAIR& b)
    {
        if (a==b)
            return true;
        if (a.second-a.first != b.second-b.first)
            return false;
        return std::equal(a.first, a.second, b.first);
    }
    static int count_identical_lines(const T*first, const T*last, int unitsperline)
    {
        auto p = first;
        auto pend = std::min(last, p+unitsperline);
        auto firstline = std::make_pair(p, pend);

        int count = 1;

        p = pend;
        while (p<last) {
            auto pend = std::min(last, p+unitsperline);
            auto thisline = std::make_pair(p, pend);

            if (!data_is_equal(firstline, thisline))
                break;

            count++;

            p = pend;
        }

        return count;
    }
    static void output_padding(std::ostream& os, int n, char fillchar)
    {
        int oneunit = 2*sizeof(T);
        if (fillchar)
            oneunit += 1;
        std::string padding(n*oneunit, ' ');
        os << padding;
    }
    static void output_asc_padding(std::ostream& os, int n)
    {
        int oneunit = sizeof(T);
        std::string padding(n*oneunit, ' ');
        os << padding;
    }


//   make it all happen
    void dump(std::ostream& os) const
    {
        // make copy of current 'os' settings
        char filler = os.fill();  // what to output between hex numbers.
        int unitsperline = os.width();  // when not set: everything on one line
        // upl == 0 -> everything on one line
        // upl ==-1 -> unspecified -> use defaults
        //int unitwidth = os.precision(); // NOT USED (yet)
        int adjust = os.flags() & os.adjustfield;
        int numberbase = os.flags() & os.basefield;

        bool showbase = os.flags() & os.showbase;
        bool showpos = os.flags() & os.showpos;
        bool showoffset = os.flags() & os.showpoint;
        bool uppercasehex = os.flags() & os.uppercase;
        bool summarize = os.flags() & os.skipws;

        uint64_t ofs = showoffset ? getbaseofs(os) : 0;
        uint64_t step = getstep(os);

        int threshold = getthreshold(os);

        // showpos adjust
        //   yes     left    %+-b     ...    invalid
        //   yes     right   %+b     asc only
        //   no      left    %-b     hex only
        //   no      right   %b      hex + asc

        if (unitsperline==-1) {
            if (adjust == os.left)
                unitsperline = 32 / sizeof(T);
            else if (showpos)
                unitsperline = 64 / sizeof(T);
            else
                unitsperline = 16 / sizeof(T);
        }

        auto p = _first;

        //auto prevp = p;
        auto prevline = std::make_pair(p, p);

        os << std::right;
        os.width(0);
        while (p < _last) {
            auto pend = unitsperline ? std::min(_last, p+unitsperline) : _last;
            auto curline = std::make_pair(p, pend);
            if (summarize) {
                if (data_is_equal(prevline, curline)) {
                    int count = count_identical_lines(p, _last, unitsperline);
                    if (count > threshold) {
                        os << "* [ 0x" << std::hex << count << " lines ]\n";
                        pend = p + count * unitsperline;

                        goto next;
                    }
                }
                else {
                    prevline = curline;
                }
            }

            if (showoffset)
                os << std::setw(8) << std::setfill('0') << std::hex << ofs << ": ";
            if (!showpos) {
                output_hex(os, curline.first, curline.second, filler, numberbase, showbase, uppercasehex);
                if (unitsperline && pend-p != unitsperline)
                    output_padding(os, unitsperline-(pend-p), filler);
            }

            if (!showpos && adjust != os.left)
                os << "  ";   // separate left from right
            if (adjust != os.left)
            {
                output_asc(os, curline.first, curline.second);
                if (unitsperline && pend-p != unitsperline)
                    output_asc_padding(os, unitsperline-(pend-p));
            }

            if (unitsperline)
                os << "\n";

        next:
            if (step) {
                ofs += step;
                p += step;
            }
            else {
                ofs += sizeof(T) * (pend-p);
                p = pend;
            }
        }
        clearflags(os);
    }

    static void output_bin(std::ostream& os, std::remove_const_t<T> val)
    {
        char bits[257];
        int len = 0;
        while (val && len<256) {
            bits[len++] = (val&1) ? '1' : '0';
            val >>= 1;
        }
        bits[len] = 0;

        int width = os.width();
        char fill = os.fill();
        if (!fill) fill = ' ';
        if (width>len) {
            os << std::setw(width-len);
            os << fill;
        }
        else {
            os << std::setw(0);
        }

        for (int i=len-1 ; i>=0 ; i--)
            os << bits[i];
    }

    template<typename INT>
    static std::enable_if_t<(sizeof(INT)<8), void> output_hex_int(std::ostream& os, INT val)
    {
        os << (((unsigned)val)&((1LL<<(8*sizeof(T)))-1));
    }
    template<typename INT>
    static std::enable_if_t<(sizeof(INT)==8), void> output_hex_int(std::ostream& os, INT val)
    {
        os << (uint64_t)val;
    }

    static void output_hex(std::ostream& os, const T*first, const T*last, char filler, int numberbase, bool showbase, bool uppercasehex)
    {
        const T* p = first;
        while (p < last)
        {
            if (filler && p > first)
                os << filler;
            auto val = *p;
            os << std::setfill('0');
            if (showbase)
                os << std::showbase;
            if (uppercasehex)
                os << std::uppercase;

            if (numberbase==os.hex) {
                os << std::setw(sizeof(T)*2);
                os << std::hex;
            }
            else if (numberbase==os.oct) {
                os << std::setw((sizeof(T)*8+2)/3);
                os << std::oct;
            }
            else if (numberbase==os.dec) {
                os << std::setw(sizeof(T)*2);
                os << std::dec;
            }

            if (getbin(os)) {
                if (showbase)
                    os << "0b";
                os << std::setw(sizeof(T)*8);
                output_bin(os, val);
            }
            else {
                output_hex_int(os, val);
            }

            ++p;
        }
    }
    static bool isprintable(char c)
    {
        return (c>=0x20 && c<=0x7e)/* || uint8_t(c)>=0xa0 */;
    }
    static void output_asc(std::ostream& os, const T*first, const T*last)
    {
        const uint8_t* p = (const uint8_t*)first;
        while (p < (const uint8_t*)last)
        {
            char c = *p;
            os << (isprintable(c) ? c : '.');

            ++p;
        }
    }


    friend std::ostream& operator<<(std::ostream&os, const Hexdumper<T>& hd)
    {
        hd.dump(os);

        // reset stream settings
        os.width(0);
        os.fill(0);
        os.precision(0);
        os.flags(std::ios_base::fmtflags(0));
        return os;
    }

};


// **************************************** //
// ********* custom io manipulators ******* //
// **************************************** //


// os << Hex::hexstring << Hex::dumper(v);
// will output the contents of v, in one line, without separators.
template <class _CharT, class _Traits>
std::basic_ostream<_CharT, _Traits>&
hexstring(std::basic_ostream<_CharT, _Traits>&os)
{
    os.fill(0);      // no separator
    os.width(0);     // one line
    os << std::left; // only hex
    os << std::hex;
    return os;
}

// os << Hex::ascstring << Hex::dumper(v);
// will output the contents of v, in one line, as simplified ascii.
template <class _CharT, class _Traits>
std::basic_ostream<_CharT, _Traits>&
ascstring(std::basic_ostream<_CharT, _Traits>&os)
{
    os.fill(0);        // no separator
    os.width(0);       // one line
    os << std::showpos;  // only ascii
    os << std::hex;
    return os;
}

// os << Hex::multiline << Hex::dumper(v);
// will output the contents of v, in a traditional multiline hexdump.
template <class _CharT, class _Traits>
std::basic_ostream<_CharT, _Traits>&
multiline(std::basic_ostream<_CharT, _Traits>&os)
{
    os.fill(' ');      // separator = SPACE
    os.width(-1);      // auto format
    // todo: clear adjust?
    os << std::hex;
    return os;
}

template <class _CharT, class _Traits>
std::basic_ostream<_CharT, _Traits>&
singleline(std::basic_ostream<_CharT, _Traits>&os)
{
    os.fill(' ');      // separator = SPACE
    os.width(0);       // single line
    // todo: clear adjust?
    os << std::hex;
    return os;
}

template <class _CharT, class _Traits>
std::basic_ostream<_CharT, _Traits>&
bin(std::basic_ostream<_CharT, _Traits>&os)
{
    Hexdumper_base::setbin(os);
    return os;
}

//  use os << Hex::offset(0x1234000) << Hex::dumper(v) << endl;
//  will output a hexdump, with each line prefixed with an offset
struct offset {
    uint64_t ofs;
    offset(uint64_t ofs=0) : ofs(ofs) { }

    friend std::ostream& operator<<(std::ostream&os, offset ofs)
    {
        os << multiline;
        Hexdumper_base::setbaseofs(os, ofs.ofs);
        os << std::showpoint;
        return os;
    }
};
struct step {
    uint64_t ofs;
    step(uint64_t ofs=0) : ofs(ofs) { }

    friend std::ostream& operator<<(std::ostream&os, step ofs)
    {
        Hexdumper_base::setstep(os, ofs.ofs);
        return os;
    }
};
struct summarize_threshold {
    int th;
    summarize_threshold(int th) : th(th) { }

    friend std::ostream& operator<<(std::ostream&os, summarize_threshold sth)
    {
        Hexdumper_base::setthreshold(os, sth.th);
        return os;
    }
};

// ******************************************************* //
// **  convenience functions creating Hexdumper objects ** //

    
// hexdump a range of items bounded by first and last pointer.
template<typename T>
Hexdumper<T> dumper(const T*first, const T*last)
{
    return Hexdumper<T>(first, last);
}

// hexdump a range of items bounded by first pointer, and item count.
template<typename T>
Hexdumper<T> dumper(const T*data, size_t size)
{
    return Hexdumper<T>(data, data+size);
}

// hexdump the contents of a sequence.
template<typename V>
Hexdumper<typename V::value_type> dumper(const V& v)
{
    if (v.empty()) {
        typename V::value_type x{0};
        // handle the empty case separately.
        return Hexdumper<typename V::value_type>(&x, 0);
    }
    return Hexdumper<typename V::value_type>(&v[0], &v[0]+v.size());
}

// TODO: support hexdumping streams.

} // end namespace
