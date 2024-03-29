#pragma once
#include <cstdint>

struct StandardBase64 {
    static char code2char(int code)
    {
        static const char*chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
        return chars[code];
    }
    static int char2code(char ch)
    {
        // -1 : invalid char
        // -2 : padding
        // -3 : whitespace

        static const int codes[] = {
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
        return codes[(uint8_t)ch];
    }
};

struct UrlSafeBase64 {
    static char code2char(int code)
    {
        static const char*chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_=";
        return chars[code];
    }
    static char char2code(char ch)
    {
        static const int codes[] = {
            //  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
            -1,-1,-1,-1,-1,-1,-1,-1,-1,-3,-3,-1,-3,-3,-1,-1, // 0
            -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, // 1
            -3,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1, // 2
            52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-2,-1,-1, // 3
            -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14, // 4
            15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,63, // 5
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
        return codes[(uint8_t)ch];
    }
};

