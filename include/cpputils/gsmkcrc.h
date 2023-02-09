#pragma once

#include <cstdint>
#include <stddef.h>

//
// As specified in RFC2440 (extracted from SecureStorage)
//

inline uint32_t gsmkcrc24(const uint8_t *ptr, size_t len) {
  uint32_t crc = 0xb704ceL;
  while (len--) {
    crc ^= (*ptr++) << 16;
    if ( (crc <<= 1) & 0x1000000L) crc ^= 0x1864cfbL;
    if ( (crc <<= 1) & 0x1000000L) crc ^= 0x1864cfbL;
    if ( (crc <<= 1) & 0x1000000L) crc ^= 0x1864cfbL;
    if ( (crc <<= 1) & 0x1000000L) crc ^= 0x1864cfbL;
    if ( (crc <<= 1) & 0x1000000L) crc ^= 0x1864cfbL;
    if ( (crc <<= 1) & 0x1000000L) crc ^= 0x1864cfbL;
    if ( (crc <<= 1) & 0x1000000L) crc ^= 0x1864cfbL;
    if ( (crc <<= 1) & 0x1000000L) crc ^= 0x1864cfbL;
  }
  return crc & 0xffffffL;
}

inline uint32_t gsmkcrc32( const uint8_t *ptr, size_t len ) {
  uint32_t crc = 0xffffffff;
  uint8_t  b;
  int      i;

  while( len-- ) {
    b = *(ptr++);
    for (i=0; i<8; i++) {
      if ((b >> 7) ^ (crc >> 31))
        crc = (crc << 1) ^ 0x04c11db7;
      else
        crc = (crc << 1);
      b <<= 1;
    }
  }

  return ~crc;             /* The complement of the remainder */
}
