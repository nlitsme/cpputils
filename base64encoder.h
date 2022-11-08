#pragma once

#include <tuple>
#include <vector>
#include <string>
#include <stdexcept>
#include "b64-alphabet.h"
/*
Functions for base64 encoding and decoding.

most convenient interface:

    std::string base64_encode(const P& data)
    std::vector<uint8_t> base64_decode(const S& txt)

Where the encode function takes any kind of vector/array, and the
decode function takes any kind of string


When you want functions which don't allocate anything
the following two are useful:

std::tuple<P, S> base64_encode(P ifirst, P ilast, S ofirst, S olast)
std::tuple<S, P, bool> base64_decode(S ifirst, S ilast, P ofirst, P olast)

these take iterator(or pointer) pairs for the input and output,
and return the last used in and output iterators.
The decoder also returns a bool, indicating if the input string contained all valid base64.

 */

/*
 * encode a 3 byte chunk into 4 characters
 */
template<typename P, typename S, typename ALPHABET=StandardBase64>
auto base64_encode_chunk(P chunk, P last, S enc, bool nopadding)
{
    // note: facebook, youtube use a modified version with  tr "+/"  "-_"
    P p = chunk;
    auto b = *p++;

    int c0= b>>2;
    int c1= (b&3)<<4;
    int c2= 0x40;
    int c3= 0x40;

    if (p < last) {
        b = *p++;
        c1 |= (b&0xf0)>>4;
        c2 = (b&0x0f)<<2;

        if (p < last) {
            b = *p++;
            c2 |= (b&0xc0)>>6;
            c3 = b&0x3f;
        }
    }

    *enc++= ALPHABET::code2char(c0);
    *enc++= ALPHABET::code2char(c1);
    if (!(nopadding && c2==0x40)) *enc++= ALPHABET::code2char(c2);
    if (!(nopadding && c3==0x40)) *enc++= ALPHABET::code2char(c3);

    return std::make_tuple(p, enc);
}
/*
 * returns a tuple with:
 *  - the next unused input pointer 'p'
 *  - the next unused output pointer 'o'
 *
 * When p!=ilast  then there was not enough space in the output buffer
 * to encode all the data.
 */
template<typename P, typename S, typename ALPHABET=StandardBase64>
std::tuple<P, S> base64_encode(P ifirst, P ilast, S ofirst, S olast, bool nopadding=false)
{
    P p = ifirst;
    S o = ofirst;
    while (p < ilast && o+4 <= olast)
    {
        auto [newp, newo] = base64_encode_chunk<P, S, ALPHABET>(p, ilast, o, nopadding);
        p = newp;
        o = newo;
    }
    return { p, o };
}

/*
   returns a tuple with:
     - where the decoding stopped
     - where the next unused output ptr.
     - success flag.
 */
template<typename S, typename P, typename ALPHABET=StandardBase64>
std::tuple<S, P, bool> base64_decode(S ifirst, S ilast, P ofirst, P olast)
{
    S p = ifirst;
    P o = ofirst;

    uint8_t b = 0;
    int i = 0;
    while (p < ilast && o < olast)
    {
        auto c = *p++;
        int cv = ALPHABET::char2code(c);
        if (cv==-2) {
            // '=' always ends a base64 encoding
            break;
        }
        else if (cv==-3) {
            // skip whitespace
            continue;
        }
        else if (cv==-1) {
            // invalid character.
            return { p, o, false };
        }

        switch(i)
        {
            case 0: b = cv<<2; i=1; break;
            case 1: b |= cv>>4; *o++ = b; b = cv<<4; i=2; break;
            case 2: b |= cv>>2; *o++ = b; b = cv<<6; i=3; break;
            case 3: b |= cv; *o++ = b; i = 0; break;
        }
    }
    // skip trailing '='
    while (p < ilast && *p == '=')
        p++;

    return { p, o, i!=1 };
}

template<typename P>
std::vector<uint8_t> base64_decode(P first, P last)
{
    std::vector<uint8_t> data(((std::distance(first, last)+3)/4)*3);
    auto [p, o, ok] = base64_decode(first, last, data.begin(), data.end());
    if (!ok)
        throw std::runtime_error("base64_decode");
    // todo: check if all data was decoded.
    //  -- problem: since decode will return when all bytes in the output are used.
    //           there still may be unprocessed space chars left.
    //     I could work around this by making the data vector one larger then needed.
    data.erase(o, data.end());
    return data;
}

template<typename S>
std::vector<uint8_t> base64_decode(const S& txt)
{
    return base64_decode(txt.begin(), txt.end());
}

template<typename P>
std::string base64_encode(const P& data)
{
    std::string txt(((data.size()+2)/3)*4, char(0));
    auto [p, o] = base64_encode(data.begin(), data.end(), txt.begin(), txt.end(), /*nopadding*/false);
    // check if all data was encoded.
    if (p != data.end())
        throw std::runtime_error("base64_encode");
    txt.erase(o, txt.end());
    return txt;
}

template<typename P>
std::string base64_encode_unpadded(const P& data)
{
    std::string txt(((data.size()+2)/3)*4, char(0));
    auto [p, o] = base64_encode(data.begin(), data.end(), txt.begin(), txt.end(), /*nopadding*/true);
    // check if all data was encoded.
    if (p != data.end())
        throw std::runtime_error("base64_encode_unpadded");
    txt.erase(o, txt.end());
    return txt;
}
