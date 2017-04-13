#ifndef __MISC_UTFCVUTILS_H__
#define __MISC_UTFCVUTILS_H__

#ifndef _WIN32
#include <sys/types.h>
#endif
#include <stdint.h>

typedef uint8_t utf8char_t;
typedef uint16_t utf16char_t;
typedef uint32_t utf32char_t;

// note:
//   the XtoYbytesneeded(const X*)  functions return a byte count
//   the XtoY(const X*, const Y*, size_t max)  conversion functions return the nr of 'X' used.
//                         and expect 'max' to include the terminating NUL
//   the Xcharcount(const X*)  functions return the nr of symbols

#define AUTOSIZE static_cast<size_t>(-1)
size_t utf8toutf32bytesneeded(const utf8char_t *p);
size_t utf8toutf32(const utf8char_t *p8, utf32char_t *p32, size_t maxsize=AUTOSIZE);

size_t utf32toutf8bytesneeded(const utf32char_t *p);
size_t utf32toutf8(const utf32char_t *p32, utf8char_t *p8, size_t maxsize=AUTOSIZE);

size_t utf16toutf32bytesneeded(const utf16char_t *p);
size_t utf16toutf32(const utf16char_t *p16, utf32char_t *p32, size_t maxsize=AUTOSIZE);

size_t utf32toutf16bytesneeded(const utf32char_t *p);
size_t utf32toutf16(const utf32char_t *p32, utf16char_t *p16, size_t maxsize=AUTOSIZE);

size_t utf8toutf16bytesneeded(const utf8char_t *p);
size_t utf8toutf16(const utf8char_t *p8, utf16char_t *p16, size_t maxsize=AUTOSIZE);

size_t utf16toutf8bytesneeded(const utf16char_t *p);
size_t utf16toutf8(const utf16char_t *p16, utf8char_t *p8, size_t maxsize=AUTOSIZE);

size_t utf8charcount(const utf8char_t *p);
size_t utf32charcount(const utf32char_t *p);
size_t utf16charcount(const utf16char_t *p);

int utf8stringcompare(const utf8char_t *l, const utf8char_t *r);
int utf16stringcompare(const utf16char_t *l, const utf16char_t *r);
int utf32stringcompare(const utf32char_t *l, const utf32char_t *r);

#endif
