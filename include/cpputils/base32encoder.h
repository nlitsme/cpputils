#pragma once

#include <tuple>
#include <vector>
#include <string>
#include <stdexcept>
#include <cstdint>

// encode a 5 byte chunk into 8 characters
template<typename P, typename S>
P base32_encode_chunk(P chunk, P last, S enc)
{
    static const char*b32ToChar= "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567=";

    P p = chunk;
    auto b = *p++;

    int c[8] = { (b&0xF8)>>3, (b&0x07)<<2, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20 };

    if (p < last) {
        b = *p++;
        c[1] |= (b&0xc0)>>6;
        c[2] = (b&0x3e)>>1;
        c[3] = (b&1)<<4;

        if (p < last) {
            b = *p++;
            c[3] |= (b&0xf0)>>4;
            c[4] = (b&0x0f)<<1;
            if (p < last) {
                b = *p++;
                c[4] |= (b&0x80)>>7;
                c[5] = (b&0x7c)>>2;
                c[6] = (b&0x03)<<3;
                if (p < last) {
                    b = *p++;
                    c[6] |= (b&0xe0)>>5;
                    c[7] = (b&0x1f);
                }
            }
        }
    }

    for (int i=0 ; i<8 ; i++)
        *enc++= b32ToChar[c[i]];

    return p;
}
template<typename P, typename S>
std::tuple<P, S> base32_encode(P ifirst, P ilast, S ofirst, S olast)
{
    P p = ifirst;
    S o = ofirst;
    while (p < ilast && o+8 <= olast)
    {
        p = base32_encode_chunk(p, ilast, o);
        o += 8;
    }
    return { p, o };
}

// -1 : invalid b32
// -2 : b32 terminator
// -3 : whitespace
// >=0: valid b32
static const int char2b32[]= {
//  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
-1,-1,-1,-1,-1,-1,-1,-1,-1,-3,-3,-1,-3,-3,-1,-1, // 0
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, // 1
-3,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, // 2
-1,-1,26,27,28,29,30,31,-1,-1,-1,-1,-1,-2,-1,-1, // 3
-1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14, // 4
15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1, // 5
-1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14, // 4
15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1, // 5
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, // 8
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, // 9
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, // a
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, // b
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, // c
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, // d
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, // e
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, // f
};
template<typename S, typename P>
std::tuple<S, P, bool> base32_decode(S ifirst, S ilast, P ofirst, P olast)
{
    S p = ifirst;
    P o = ofirst;

    uint8_t b = 0;
    int i = 0;
    while (p < ilast && o < olast)
    {
        auto c = *p++;
        int cv = char2b32[(uint8_t)c];
        if (cv==-2) {
            // '=' is always a proper base32 ending
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

        // 00000111 11222223 33334444 45555566 66677777
        switch(i)
        {
            case 0: b = cv<<3; i=1; break;
            case 1: b |= cv>>2; *o++ = b; b = cv<<6; i=2; break;
            case 2: b |= cv<<1; i=3; break;
            case 3: b |= cv>>4; *o++ = b; b = cv<<4; i=4; break;
            case 4: b |= cv>>1; *o++ = b; b = cv<<7; i=5; break;
            case 5: b |= cv<<2; i=6; break;
            case 6: b |= cv>>3; *o++ = b; b = cv<<5; i=7; break;
            case 7: b |= cv; *o++ = b; i = 0; break;
        }
    }
    // skip trailing '='
    while (p < ilast && *p == '=')
        p++;

    return { p, o, i==0 };
}

template<typename S>
std::vector<uint8_t> base32_decode(const S& txt)
{
    std::vector<uint8_t> data(((txt.size()+7)/8)*5);
    auto [p, o, ok] = base32_decode(txt.begin(), txt.end(), data.begin(), data.end());
    if (!ok)
        throw std::runtime_error("base32_decode");
    // todo: check if all data was decoded.
    data.erase(o, data.end());
    return data;
}

template<typename P>
std::string base32_encode(const P& data)
{
    std::string txt(((data.size()+4)/5)*8, char(0));
    auto [p, o] = base32_encode(data.begin(), data.end(), txt.begin(), txt.end());
    // todo: check if all data was encoded.
    txt.erase(o, txt.end());
    return txt;
}

