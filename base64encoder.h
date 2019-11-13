/*
/Users/itsme/myprj/itslib/include/itslib/basexxdecoder.h
/Users/itsme/myprj/itslib/include/itslib/str/b64.h
/Users/itsme/myprj/itslib/include/itslib/stringutils.h
/Users/itsme/myprj/itslib/src/stringutils.cpp
/Users/itsme/workprj/draupnir/sindri/cli/utils.cpp
/Users/itsme/workprj/secphone/libraries/SecureStorage/base64.h
/Users/itsme/workprj/secphone/libraries/SecureStorage/base64.cpp
/Users/itsme/workprj/secphone/libraries/miscutils/misc/base64conv.h
/Users/itsme/workprj/secphone/libraries/miscutils/misc/base64conv.cpp
*/


template<typename P, typename S>
P base64_encode_chunk(P chunk, P last, S enc)
{
    // note: facebook, youtube use a modified version with  tr "+/"  "-_"
    static const char*b642char= "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

    P p = chunk;
    auto b = *p++;

    int c0= b>>2;
    int c1= (b&3)<<4;
    int c2= 0;
    int c3= 0;

    if (p < last) {
        b = *p++;
        c1 |= (b&0xf0)>>4;
        c2 |= (b&0x0f)<<2;
    }
    else {
        c2=64;
    }

    if (p < last) {
        b = *p++;
        c2 |= (b&0xc0)>>6;
        c3 |= b&0x3f;
    }
    else {
        c3=64;
    }

    *enc++= b642char[c0];
    *enc++= b642char[c1];
    *enc++= b642char[c2];
    *enc++= b642char[c3];

    return p;
}
template<typename P, typename S>
std::tuple<P, S> base64_encode(P ifirst, P ilast, S ofirst, S olast)
{
    P p = ifirst;
    S o = ofirst;
    while (p < ilast && o+4 <= olast)
    {
        p = base64_encode_chunk(p, ilast, o);
        o += 4;
    }
    return { p, o };
}

// -1 : invalid b64
// -2 : b64 terminator
// -3 : whitespace
// >=0: valid b64
static const int char2b64[]= {
//  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
-1,-1,-1,-1,-1,-1,-1,-1,-1,-3,-3,-1,-3,-3,-1,-1, // 0
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, // 1
-3,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63, // 2
52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-2,-1,-1, // 3
-1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14, // 4
15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1, // 5
-1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40, // 6
41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1, // 7
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
std::tuple<S, P, bool> base64_decode(S ifirst, S ilast, P ofirst, P olast)
{
    S p = ifirst;
    P o = ofirst;

    uint8_t b;
    int i = 0;
    while (p < ilast && o < olast)
    {
        auto c = *p++;
        int cv = char2b64[(uint8_t)c];
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

template<typename S>
std::vector<uint8_t> base64_decode(const S& txt)
{
    std::vector<uint8_t> data(((txt.size()+3)/4)*3);
    auto [p, o, ok] = base64_decode(txt.begin(), txt.end(), data.begin(), data.end());
    if (!ok)
        throw std::runtime_error("base64_decode");
    // todo: check if all data was decoded.
    data.erase(o, data.end());
    return data;
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
