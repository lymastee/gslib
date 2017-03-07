/*
 * Copyright (c) 2016-2017 lymastee, All rights reserved.
 * Contact: lymastee@hotmail.com
 *
 * This file is part of the gslib project.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <gslib/md5.h>

__gslib_begin__

static const uint
    s11 =  7, s12 = 12, s13 = 17, s14 = 22,
    s21 =  5, s22 =  9, s23 = 14, s24 = 20,
    s31 =  4, s32 = 11, s33 = 16, s34 = 23,
    s41 =  6, s42 = 10, s43 = 15, s44 = 21;

static inline uint fx(uint x, uint y, uint z) { return (x & y) | (~x & z); }
static inline uint gx(uint x, uint y, uint z) { return (x & z) | (y & ~z); }
static inline uint hx(uint x, uint y, uint z) { return x ^ y ^ z; }
static inline uint ix(uint x, uint y, uint z) { return y ^ (x | ~z); }
static inline uint rol(uint x, uint n) { return (x << n) | (x >> (32-n)); }

static inline void ff(uint& a, uint b, uint c, uint d, uint x, uint s, uint ac)
{
    a += fx(b, c, d) + x + ac;
    a = rol(a, s);
    a += b;
}

static inline void gg(uint& a, uint b, uint c, uint d, uint x, uint s, uint ac)
{
    a += gx(b, c, d) + x + ac;
    a = rol(a, s);
    a += b;
}

static inline void hh(uint& a, uint b, uint c, uint d, uint x, uint s, uint ac)
{
    a += hx(b, c, d) + x + ac;
    a = rol(a, s);
    a += b;
}

static inline void ii(uint& a, uint b, uint c, uint d, uint x, uint s, uint ac)
{
    a += ix(b, c, d) + x + ac;
    a = rol(a, s);
    a += b;
}

static inline gchar hexconv(uint x)
{
    if(x >= 0 && x < 10)
        return x + _t('0');
    else if(x >= 10 && x < 16)
        return x - 10 + _t('A');
    return 0;
}

void md5::reset()
{
    _count[0] = 0;
    _count[1] = 0;
    _state[0] = 0x67452301;
    _state[1] = 0xefcdab89;
    _state[2] = 0x98badcfe;
    _state[3] = 0x10325476;
}

void md5::update(const byte* src, int len)
{
    uint i = 0, index, l;
    index = (uint)((_count[0] >> 3) & 0x3f);
    if((_count[0] += ((uint)len << 3)) < ((uint)len << 3))
        _count[1] ++;
    _count[1] += ((uint)len >> 29);
    l = 64 - index;
    if((uint)len >= l) {
        memcpy_s(_buffer+index, sizeof(_buffer), src, l);
        transform(_buffer);
        for(i = l; i + 63 < (uint)len; i += 64)
            transform(src+i);
        index = 0;
    }
    memcpy_s(_buffer+index, sizeof(_buffer), src+i, len-i);
}

void md5::get_string(string& str) const
{
    const byte* r = _digest;
    str.clear();
    for(int i = 0; i < _countof(_digest); i ++) {
        str.push_back(hexconv(r[i]>>4));
        str.push_back(hexconv(r[i]&0xf));
    }
}

void md5::finalize()
{
    static const byte pad[64] = { 0x80 };
    byte bits[8];
    uint os[4], oc[2];
    uint index, l;
    memcpy_s(os, sizeof(os), _state, sizeof(_state));
    memcpy_s(oc, sizeof(oc), _count, sizeof(_count));
    encode(_count, bits, 2);
    index = (uint)(_count[0] >> 3) & 0x3f;
    l = index < 56 ? 56 - index : 120 - index;
    update(pad, l);
    update(bits, 8);
    encode(_state, _digest, 4);
    memcpy_s(_state, sizeof(_state), os, sizeof(os));
    memcpy_s(_count, sizeof(_count), oc, sizeof(oc));
}

void md5::transform(const byte block[64])
{
    uint a = _state[0], b = _state[1], c = _state[2], d = _state[3], x[16];
    decode(block, x, _countof(x));

    ff(a, b, c, d, x[0],  s11, 0xd76aa478);
    ff(d, a, b, c, x[1],  s12, 0xe8c7b756);
    ff(c, d, a, b, x[2],  s13, 0x242070db);
    ff(b, c, d, a, x[3],  s14, 0xc1bdceee);
    ff(a, b, c, d, x[4],  s11, 0xf57c0faf);
    ff(d, a, b, c, x[5],  s12, 0x4787c62a);
    ff(c, d, a, b, x[6],  s13, 0xa8304613);
    ff(b, c, d, a, x[7],  s14, 0xfd469501);
    ff(a, b, c, d, x[8],  s11, 0x698098d8);
    ff(d, a, b, c, x[9],  s12, 0x8b44f7af);
    ff(c, d, a, b, x[10], s13, 0xffff5bb1);
    ff(b, c, d, a, x[11], s14, 0x895cd7be);
    ff(a, b, c, d, x[12], s11, 0x6b901122);
    ff(d, a, b, c, x[13], s12, 0xfd987193);
    ff(c, d, a, b, x[14], s13, 0xa679438e);
    ff(b, c, d, a, x[15], s14, 0x49b40821);

    gg(a, b, c, d, x[1],  s21, 0xf61e2562);
    gg(d, a, b, c, x[6],  s22, 0xc040b340);
    gg(c, d, a, b, x[11], s23, 0x265e5a51);
    gg(b, c, d, a, x[0],  s24, 0xe9b6c7aa);
    gg(a, b, c, d, x[5],  s21, 0xd62f105d);
    gg(d, a, b, c, x[10], s22,  0x2441453);
    gg(c, d, a, b, x[15], s23, 0xd8a1e681);
    gg(b, c, d, a, x[4],  s24, 0xe7d3fbc8);
    gg(a, b, c, d, x[9],  s21, 0x21e1cde6);
    gg(d, a, b, c, x[14], s22, 0xc33707d6);
    gg(c, d, a, b, x[3],  s23, 0xf4d50d87);
    gg(b, c, d, a, x[8],  s24, 0x455a14ed);
    gg(a, b, c, d, x[13], s21, 0xa9e3e905);
    gg(d, a, b, c, x[2],  s22, 0xfcefa3f8);
    gg(c, d, a, b, x[7],  s23, 0x676f02d9);
    gg(b, c, d, a, x[12], s24, 0x8d2a4c8a);

    hh(a, b, c, d, x[5],  s31, 0xfffa3942);
    hh(d, a, b, c, x[8],  s32, 0x8771f681);
    hh(c, d, a, b, x[11], s33, 0x6d9d6122);
    hh(b, c, d, a, x[14], s34, 0xfde5380c);
    hh(a, b, c, d, x[1],  s31, 0xa4beea44);
    hh(d, a, b, c, x[4],  s32, 0x4bdecfa9);
    hh(c, d, a, b, x[7],  s33, 0xf6bb4b60);
    hh(b, c, d, a, x[10], s34, 0xbebfbc70);
    hh(a, b, c, d, x[13], s31, 0x289b7ec6);
    hh(d, a, b, c, x[0],  s32, 0xeaa127fa);
    hh(c, d, a, b, x[3],  s33, 0xd4ef3085);
    hh(b, c, d, a, x[6],  s34,  0x4881d05);
    hh(a, b, c, d, x[9],  s31, 0xd9d4d039);
    hh(d, a, b, c, x[12], s32, 0xe6db99e5);
    hh(c, d, a, b, x[15], s33, 0x1fa27cf8);
    hh(b, c, d, a, x[2],  s34, 0xc4ac5665);

    ii(a, b, c, d, x[0],  s41, 0xf4292244);
    ii(d, a, b, c, x[7],  s42, 0x432aff97);
    ii(c, d, a, b, x[14], s43, 0xab9423a7);
    ii(b, c, d, a, x[5],  s44, 0xfc93a039);
    ii(a, b, c, d, x[12], s41, 0x655b59c3);
    ii(d, a, b, c, x[3],  s42, 0x8f0ccc92);
    ii(c, d, a, b, x[10], s43, 0xffeff47d);
    ii(b, c, d, a, x[1],  s44, 0x85845dd1);
    ii(a, b, c, d, x[8],  s41, 0x6fa87e4f);
    ii(d, a, b, c, x[15], s42, 0xfe2ce6e0);
    ii(c, d, a, b, x[6],  s43, 0xa3014314);
    ii(b, c, d, a, x[13], s44, 0x4e0811a1);
    ii(a, b, c, d, x[4],  s41, 0xf7537e82);
    ii(d, a, b, c, x[11], s42, 0xbd3af235);
    ii(c, d, a, b, x[2],  s43, 0x2ad7d2bb);
    ii(b, c, d, a, x[9],  s44, 0xeb86d391);

    _state[0] += a;
    _state[1] += b;
    _state[2] += c;
    _state[3] += d;
}

void md5::encode(const uint* src, byte* des, int len)
{
    for(int i = 0; i < len; i ++, src ++) {
        *des++ = (byte)(*src & 0xff);
        *des++ = (byte)((*src >> 8) & 0xff);
        *des++ = (byte)((*src >> 16) & 0xff);
        *des++ = (byte)((*src >> 24) & 0xff);
    }
}

void md5::decode(const byte* src, uint* des, int len)
{
    for(int i = 0; i < len; i ++, des ++) {
        *des = (uint)(*src++);
        *des |= (uint)(*src++) << 8;
        *des |= (uint)(*src++) << 16;
        *des |= (uint)(*src++) << 24;
    }
}

__gslib_end__
