#include <stddef.h>
#include <stdint.h>
#include "utfcvutils.h"

// relevant rfcs:
//  rfc 2781  utf16
//  rfc 3629  utf8
//  rfc 2152  utf7      
//  rfc 2279  utf8 (obsolete)
//  rfc 2044  utf8 (obsolete)
//  rfc 1642  utf7 (obsolete)
//  rfc 4042  utf9+utf18 (april 1st)
// todo: remember start of current input symbol, so we can return the 
// correct nr of items used when the output buf is too small
//
// todo: add interface to return status, used-input, used-output

// utf8                                                                                      utf16                                                   utf32
// 00000000-0000007F | 0xxxxxxx                            | 00-7f                         | 00xx         | 00000000 0xxxxxxx                       | 0000 00xx    | 00000000 00000000 00000000 0xxxxxxx 
// 00000080-000007FF | 110xxxxx 10xxxxxx                   | c0-df 80-bf                   | 0xxx         | 00000xxx xxxxxxxx                       | 0000 0xxx    | 00000000 00000000 00000xxx xxxxxxxx
// 00000800-0000FFFF | 1110xxxx 10xxxxxx 10xxxxxx          | e0-ef 80-bf 80-bf             | xxxx         | xxxxxxxx xxxxxxxx                       | 0000 xxxx    | 00000000 00000000 xxxxxxxx xxxxxxxx
// 00010000-0010FFFF | 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx | f0-f7 80-bf 80-bf 80-bf       | d8yy dcxx    | 110110yy yyyyyyyy  110111xx xxxxxxxx    | d8yy dcxx    | 00000000 0001xxxx xxxxxxxx xxxxxxxx
//
// the obsolete rfc 2279 also specified these ranges:
// 00200000-03FFFFFF | 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
// 04000000-7FFFFFFF | 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx

// utf16
// d800-dbff   110110yy yyyyyyyy  high half
// dc00-dfff   110111xx xxxxxxxx  low half
//    ->  yyyy yyyyyyxx xxxxxxxx   + 0x10000



/////////////////////////////////////////////////////////
//  <UTFNN>to<UTFNN>bytesneeded: calculate how many bytes exactly a conversion will need
//
//  <UTFNN>charcount:            calculate how many symbols there are in a string
//
//  <UTFNN>to<UTFNN>             convert between utf representations, return nr of code points used.
//                               result is terminated with NUL symbol.
//

/*
 *  todo:
 *  add failure policy argument:
 *     - ignore  -> ignores bad code points
 *     - copy    -> when possible just copy the bad code points
 *     - replace -> replace bad code points with '?'
 *     - throw   -> throw an exception
 *     - abort   -> return truncated string
 *
 */

size_t utf8bytesneeded(utf32char_t c)
{
    if (c<0x80) return 1;
    if (c<0x800) return 2;
    if (c<0x10000) return 3;
    return 4; // < 0x110000
}
size_t utf32toutf8bytesneeded(const utf32char_t *p)
{
    size_t n=0;
    while (*p)
        n += utf8bytesneeded(*p++);
    return n;
}
size_t utf16toutf8bytesneeded(const utf16char_t *p)
{
    size_t n=0;
    utf32char_t w=0;
    while (uint16_t c= *p++) {
        if (c<0xd800 || c>=0xe000)
            n += utf8bytesneeded(c);
        else if (c<0xdc00) {
            w = c&0x3ff;
        }
        else { // dc00..e000
            w = 0x10000 + ((w<<10) | (c&0x3ff));
            n += utf8bytesneeded(w);
            w = 0;
        }
    }
    return n;
}

size_t utf8charcount(const utf8char_t *p)
{
    size_t n=0;
    while (uint8_t c= *p++)
        if (c<0x80 || c>=0xc0)
            n++;
    return n;
}
size_t utf8toutf32bytesneeded(const utf8char_t *p)
{
    return utf8charcount(p)*sizeof(utf32char_t);
}
size_t utf8toutf16bytesneeded(const utf8char_t *p)
{
    size_t n=0;
    while (uint8_t c= *p++) {
        if (c<0x80 || c>=0xc0)
            n++;
        if (c>=0xf0)
            n++;
    }
    return n*sizeof(utf16char_t);
}
size_t utf32charcount(const utf32char_t *p)
{
    size_t n=0;
    while (*p++)
        n++;
    return n;
}
size_t utf32toutf16bytesneeded(const utf32char_t *p)
{
    size_t n=0;
    while (utf32char_t c= *p++) {
        n++;
        if (c>=0x10000)
            n++;
    }
    return n*sizeof(utf16char_t);
}
template<typename T>
bool checkend(T*p, T*end, size_t size)
{
    if (size==AUTOSIZE)
        return true;
    return p<end;
}
size_t utf8toutf32(const utf8char_t *p8, utf32char_t *p32, size_t maxsize)
{
    const utf8char_t *p8start= p8;
    utf32char_t *p32end= p32+maxsize-1;
    int n=-1;
    utf32char_t w=0;
    uint8_t c;
    while (checkend(p32,p32end,maxsize) && (c= *p8)!=0)
    {
        if (c<0x80)
            *p32++ = c;
        else if (c<0xc0) {
            // break on invalid utf8
            if (n<0)
                break;

            w = (w<<6) | (c&0x3f);
            if (n==0) {

                // break on invalid codes
                if (w>=0xd800 && w<0xe000)
                    break;
                if (w>=0x110000)
                    break;

                *p32++ = w;
                n=-1;
            }
            else
                n--;
        }
        else if (c<0xe0) {
            w = (c&0x1f);
            n= 0;
        }
        else if (c<0xf0) {
            w = (c&0xf);
            n= 1;
        }
        else {  // c < 0xf8
            w = (c&0x7);
            n= 2;
        }
        ++p8;
    }
    *p32++ = 0;
    return p8-p8start;
}
size_t utf32toutf8(const utf32char_t *p32, utf8char_t *p8, size_t maxsize)
{
    const utf32char_t *p32start= p32;
    utf8char_t *p8end= p8+maxsize-1;
    utf32char_t c;
    while (checkend(p8,p8end,maxsize) && (c= *p32)!=0)
    {
        if (c<0x80)
            *p8++ = c;
        else if (c<0x800) {
            if (!checkend(p8+1,p8end,maxsize))
                break;
            *p8++ = 0xc0 + (c>>6);
            *p8++ = 0x80 + (c&0x3f);
        }
        else if (c<0x10000) {
            // break on invalid codes
            if (c>=0xd800 && c<0xe000)
                break;

            if (!checkend(p8+2,p8end,maxsize))
                break;
            *p8++ = 0xe0 + (c>>12);
            *p8++ = 0x80 + ((c>>6)&0x3f);
            *p8++ = 0x80 + (c&0x3f);
        }
        else if (c<0x110000) {
            if (!checkend(p8+3,p8end,maxsize))
                break;
            *p8++ = 0xf0 + (c>>18);
            *p8++ = 0x80 + ((c>>12)&0x3f);
            *p8++ = 0x80 + ((c>>6)&0x3f);
            *p8++ = 0x80 + (c&0x3f);
        }
        else {
            // break on invalid codes
            break;
        }

        ++p32;
    }
    *p8++ = 0;
    return p32-p32start;
}

size_t utf16toutf32(const utf16char_t *p16, utf32char_t *p32, size_t maxsize)
{
    const utf16char_t *p16start= p16;
    utf32char_t *p32end= p32+maxsize-1;
    utf32char_t w=0;
    int n=-1;
    uint16_t c;
    while (checkend(p32,p32end,maxsize) && (c= *p16)!=0)
    {
        if (c<0xd800 || c>=0xe000)
            *p32++ = c;
        else if (c<0xdc00) {
            // break on  d8xx d8xx sequences
            if (n!=-1)
                break;

            w = c&0x3ff;
            n = 0;
        }
        else { // c=dc00 .. dfff
            // break on  dcxx  without d8xx
            if (n!=0)
                break;
            w = 0x10000 + ((w<<10) | (c&0x3ff));
            *p32++ = w;

            n = -1;
        }

        ++p16;
    }
    *p32++ = 0;
    return p16-p16start;
}
size_t utf32toutf16(const utf32char_t *p32, utf16char_t *p16, size_t maxsize)
{
    const utf32char_t *p32start= p32;
    utf16char_t *p16end= p16+maxsize-1;
    utf32char_t c;
    while (checkend(p16,p16end,maxsize) && (c= *p32)!=0)
    {
        if (c<0x10000) {
            // break on invalid codes
            if (c>=0xd800 && c<0xe000)
                break;
            *p16++ = c;
        }
        else if (c<=0x110000) {
            if (!checkend(p16+1,p16end,maxsize))
                break;
            c -= 0x10000;
            *p16++ = 0xd800+(c>>10);
            *p16++ = 0xdc00+(c&0x3ff);
        }
        else {
            // break on invalid codes
            break;
        }

        ++p32;
    }
    *p16++ = 0;
    return p32-p32start;
}
size_t utf8toutf16(const utf8char_t *p8, utf16char_t *p16, size_t maxsize)
{
    const utf8char_t *p8start= p8;
    utf16char_t *p16end= p16+maxsize-1;
    int n=-1;
    utf32char_t w=0;
    uint8_t c;
    while (checkend(p16,p16end,maxsize) && (c= *p8)!=0)
    {
        if (c<0x80)
            *p16++ = c;
        else if (c<0xc0) {
            // break on invalid utf8
            if (n<0)
                break;

            w = (w<<6) | (c&0x3f);
            if (n==0) {
                // break on invalid codes
                if (w>=0xd800 && w<0xe000)
                    break;
                if (w>=0x110000)
                    break;

                if (w<0x10000)
                    *p16++ = w;
                else {
                    if (!checkend(p16+1,p16end,maxsize))
                        break;
                    w -= 0x10000;
                    *p16++ = 0xd800+(w>>10);
                    *p16++ = 0xdc00+(w&0x3ff);
                }
                n=-1;
            }
            else
                n--;
        }
        else if (c<0xe0) {
            w = (c&0x1f);
            n= 0;
        }
        else if (c<0xf0) {
            w = (c&0xf);
            n= 1;
        }
        else {  // c < 0xf8
            w = (c&0x7);
            n= 2;
        }

        ++p8;
    }
    *p16++ = 0;
    return p8-p8start;
}
size_t utf16toutf8(const utf16char_t *p16, utf8char_t *p8, size_t maxsize)
{
    const utf16char_t *p16start= p16;
    utf8char_t *p8end= p8+maxsize-1;
    utf32char_t w=0;
    int n=-1;
    bool emit=false;
    uint16_t c;
    while (checkend(p8,p8end,maxsize) && (c= *p16)!=0)
    {
        if ((c<0xd800) || (c>=0xe000)) {
            w = c;
            emit= true;
        }
        else if (c<0xdc00) {
            // break on  d8xx d8xx sequences
            if (n!=-1)
                break;

            w = c&0x3ff;
            emit= false;
            n = 0;
        }
        else { // c=dc00 .. dfff
            // break on  dcxx  without d8xx
            if (n!=0)
                break;

            w = 0x10000 + ((w<<10) | (c&0x3ff));
            emit= true;

            n = -1;
        }
        if (emit) {
            if (w<0x80) {
                *p8++ = w;
            }
            else if (w<0x800) {
                if (!checkend(p8+1,p8end,maxsize))
                    break;
                *p8++ = 0xc0 + (w>>6);
                *p8++ = 0x80 + (w&0x3f);
            }
            else if (w<0x10000) {
                if (!checkend(p8+2,p8end,maxsize))
                    break;
                *p8++ = 0xe0 + (w>>12);
                *p8++ = 0x80 + ((w>>6)&0x3f);
                *p8++ = 0x80 + (w&0x3f);
            }
            else {
                if (!checkend(p8+3,p8end,maxsize))
                    break;
                *p8++ = 0xf0 + (w>>18);
                *p8++ = 0x80 + ((w>>12)&0x3f);
                *p8++ = 0x80 + ((w>>6)&0x3f);
                *p8++ = 0x80 + (w&0x3f);
            }
        }

        ++p16;
    }
    *p8++ = 0;
    return p16-p16start;
}

size_t utf16bytesneeded(utf32char_t c)
{
    if (c<0x10000) return 2;
    return 4;   // < 0x110000
}
size_t utf16bytesneeded(const utf32char_t *p)
{
    size_t n=0;
    while (*p)
        n += utf16bytesneeded(*p++);
    return n;
}
size_t utf16charcount(const utf16char_t *p)
{
    size_t n=0;
    while (uint16_t c= *p++)
        if (c<0xdc00 || c>=0xe000)
            n++;
    return n;
}
size_t utf16toutf32bytesneeded(const utf16char_t *p)
{
    return utf16charcount(p)*sizeof(utf32char_t);
}
const utf8char_t* getutf8(const utf8char_t *p, utf32char_t& val)
{
    utf8char_t b0= *p++;
    utf32char_t c;
    int n;
    if (b0>=0xf0) {
        c= b0&7;
        n=3;
    }
    else if (b0>=0xe0) {
        c= b0&15;
        n=2;
    }
    else if (b0>=0xc0) {
        c= b0&31;
        n=1;
    }
    else if (b0>=0x80) {
        // invalid utf8
        return NULL;
    }
    else {
        val= b0;
        return p;
    }
    while (n--) {
        c <<= 6;
        utf8char_t b= *p++;
        c |= b&0x3f;
        if ( (b&0xc0)!=0x80 ) {
            // invalid utf8
            return NULL;
        }
    }
    val = c;
    return p;
}
const utf16char_t* getutf16(const utf16char_t *p, utf32char_t& val)
{
    utf16char_t b0= *p++;
    if (b0>=0xd800 && b0<0xe000) {
        utf16char_t b1= *p++;
        if (b0>=0xdc00)
            return NULL;
        if (b1<0xdc00 || b1>=0xe000)
            return NULL;

        val= b0&0x3ff;
        val<<=10;
        val|= b1&0x3ff;
        val+= 0x10000;
        return p;
    }
    val= b0;
    return p;
}
int utf8stringcompare(const utf8char_t *l, const utf8char_t *r)
{
    while (*l && *r)
    {
        utf32char_t lval; l = getutf8(l, lval);
        utf32char_t rval; r = getutf8(r, rval);
        if (!l || !r)
            break;
        if (lval<rval)
            return -1;
        if (lval>rval)
            return 1;
    }
    return 0;
}
int utf16stringcompare(const utf16char_t *l, const utf16char_t *r)
{
    while (*l && *r)
    {
        utf32char_t lval; l = getutf16(l, lval);
        utf32char_t rval; r = getutf16(r, rval);
        if (!l || !r)
            break;
        if (lval<rval)
            return -1;
        if (lval>rval)
            return 1;
    }
    return 0;
}
int utf32stringcompare(const utf8char_t *l, const utf8char_t *r)
{
    while (*l && *r)
    {
        if (*l < *r)
            return -1;
        if (*l > *r)
            return 1;
        l++;
        r++;
    }
    return 0;
}
