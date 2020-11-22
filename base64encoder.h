#pragma once

#include <tuple>
#include <vector>
#include <string>
#include "b64-alphabet.h"

// encode a 3 byte chunk into 4 characters
template<typename P, typename S, typename ALPHABET=StandardBase64>
P base64_encode_chunk(P chunk, P last, S enc)
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
    *enc++= ALPHABET::code2char(c2);
    *enc++= ALPHABET::code2char(c3);

    return p;
}
template<typename P, typename S, typename ALPHABET=StandardBase64>
std::tuple<P, S> base64_encode(P ifirst, P ilast, S ofirst, S olast)
{
    P p = ifirst;
    S o = ofirst;
    while (p < ilast && o+4 <= olast)
    {
        p = base64_encode_chunk<P, S, ALPHABET>(p, ilast, o);
        o += 4;
    }
    return { p, o };
}

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
            // '=' is always a proper base64 ending
            i = 0;
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

    return { p, o, i==0 };
}

template<typename P>
std::vector<uint8_t> base64_decode(P first, P last)
{
    std::vector<uint8_t> data(((std::distance(first, last)+3)/4)*3);
    auto [p, o, ok] = base64_decode(first, last, data.begin(), data.end());
    if (!ok)
        throw std::runtime_error("base64_decode");
    // todo: check if all data was decoded.
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
    auto [p, o] = base64_encode(data.begin(), data.end(), txt.begin(), txt.end());
    // todo: check if all data was encoded.
    txt.erase(o, txt.end());
    return txt;
}
