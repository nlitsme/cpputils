#pragma once
#include <array>

template<typename INT, INT poly, int nbits>
class CrcCalc {
    std::array<INT, 256> table;
    INT polymult(uint8_t b)
    {
        INT result = 0;
        INT factor = poly;
        for (int i = 0 ; i < 8 ; i++)
        {
            if (b & 0x80)
                result ^= factor;
            b <<= 1;
            bool bit = factor&1;
            factor >>= 1;
            if (bit)
                factor ^= poly;
        }
        return result;
    }
    void calc_table()
    {
        for (int i = 0 ; i < 256 ; i++)
            table[i] = polymult(i);
    }

public:
    CrcCalc()
    {
        calc_table();
    }
    INT add(INT crc, uint8_t b) const
    {
        return table[(crc^b)&0xFF] ^ (crc>>8);
    }
    INT calc(INT crc, const uint8_t *p, int size) const
    {
        constexpr INT mask = ((((INT)1<<(nbits-1))-1)<<1) | 1;
        while (size--)
            crc = add(crc, *p++);
        return crc ^ mask;
    }
};


template<typename V>
uint16_t crc16(uint16_t crc, const V& v)
{
    static CrcCalc<uint16_t, 0x8408, 16> CRC;
    return CRC.calc(crc, &v[0], v.size());
} 

template<typename P>
uint16_t crc16(uint16_t crc, P ptr, size_t size)
{
    static CrcCalc<uint16_t, 0x8408, 16> CRC;
    return CRC.calc(crc, ptr, size);
} 

template<typename V>
uint32_t crc32(uint32_t crc, const V& v)
{
    static CrcCalc<uint32_t, 0xEDB88320, 32> CRC;
    return CRC.calc(crc, &v[0], v.size());
} 

template<typename P>
uint32_t crc32(uint32_t crc, P ptr, size_t size)
{
    static CrcCalc<uint32_t, 0xEDB88320, 32> CRC;
    return CRC.calc(crc, ptr, size);
} 

