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

#include <emmintrin.h>
#include <gslib/sha1.h>

/*
 * This implementation comes from:
 * http://www.arctic.org/~dean/crypto/sha1.html
 * I did some modifications.
 */

__gslib_begin__

void sha1::reset()
{
    _digest[0] = 0x67452301;
    _digest[1] = 0xefcdab89;
    _digest[2] = 0x98badcfe;
    _digest[3] = 0x10325476;
    _digest[4] = 0xc3d2e1f0;
}

union v4si
{
    uint        u32[4];
    __m128i     u128;
};

static const v4si k00_19 = { 0x5a827999, 0x5a827999, 0x5a827999, 0x5a827999 };
static const v4si k20_39 = { 0x6ed9eba1, 0x6ed9eba1, 0x6ed9eba1, 0x6ed9eba1 };
static const v4si k40_59 = { 0x8f1bbcdc, 0x8f1bbcdc, 0x8f1bbcdc, 0x8f1bbcdc };
static const v4si k60_79 = { 0xca62c1d6, 0xca62c1d6, 0xca62c1d6, 0xca62c1d6 };

#define load(p)     _mm_loadu_si128(p)

#define prep00_15(prep, w) do { \
        __m128i r1, r2; \
        r1 = (w); \
        r1 = _mm_shufflehi_epi16(r1, _MM_SHUFFLE(2, 3, 0, 1)); \
        r1 = _mm_shufflelo_epi16(r1, _MM_SHUFFLE(2, 3, 0, 1)); \
        r2 = _mm_slli_epi16(r1, 8); \
        r1 = _mm_srli_epi16(r1, 8); \
        r1 = _mm_or_si128(r1, r2); \
        (w) = r1; \
        (prep).u128 = _mm_add_epi32(k00_19.u128, r1); \
    } while(0)

#define prep(prep, xw0, xw1, xw2, xw3, k) do { \
        __m128i r0, r1, r2, r3; \
        r3 = _mm_srli_si128((xw3), 4); \
        r0 = (xw0); \
        r1 = _mm_shuffle_epi32((xw0), _MM_SHUFFLE(1,0,3,2)); \
        r1 = _mm_unpacklo_epi64(r1, (xw1)); \
        r2 = (xw2); \
        r0 = _mm_xor_si128(r1, r0); \
        r2 = _mm_xor_si128(r3, r2); \
        r0 = _mm_xor_si128(r2, r0); \
        r2 = _mm_slli_si128(r0, 12); \
        r1 = _mm_cmplt_epi32(r0, _mm_setzero_si128()); \
        r0 = _mm_add_epi32(r0, r0); \
        r0 = _mm_sub_epi32(r0, r1); \
        r3 = _mm_srli_epi32(r2, 30); \
        r2 = _mm_slli_epi32(r2, 2); \
        r0 = _mm_xor_si128(r0, r3); \
        r0 = _mm_xor_si128(r0, r2); \
        (xw0) = r0; \
        (prep).u128 = _mm_add_epi32(r0, (k).u128); \
    } while(0)

static inline uint rol(uint src, uint amt)
{
    /* gcc and icc appear to turn this into a rotate */
    return (src << amt) | (src >> (32 - amt));
}

static inline uint f00_19(uint x, uint y, uint z)
{
    /* FIPS 180-2 says this: (x & y) ^ (~x & z)
     * but we can calculate it in fewer steps.
     */
    return ((y ^ z) & x) ^ z;
}

static inline uint f20_39(uint x, uint y, uint z)
{
    return (x ^ z) ^ y;
}

static inline uint f40_59(uint x, uint y, uint z)
{
    /* FIPS 180-2 says this: (x & y) ^ (x & z) ^ (y & z)
     * but we can calculate it in fewer steps.
     */
    return (x & z) | ((x | z) & y);
}

static inline uint f60_79(uint x, uint y, uint z)
{
    return f20_39(x, y, z);
}

#define step(nn_mm, xa, xb, xc, xd, xe, xt, input) do { \
        (xt) = (input) + f##nn_mm((xb), (xc), (xd)); \
        (xb) = rol((xb), 30); \
        (xt) += ((xe) + rol((xa), 5)); \
    } while(0)

#define rev_endian(x) (((x) >> 24) | (((x) & 0x00ff0000) >> 8) | (((x) & 0x0000ff00) << 8) | (x << 24))

static inline void rev_write(byte dest[8], uint64 data)
{
    assert(dest);
    uint hi, lo;
    lo = ((uint*)&data)[0];
    hi = ((uint*)&data)[1];
    ((uint*)dest)[0] = rev_endian(hi);
    ((uint*)dest)[1] = rev_endian(lo);
}

static inline void update_once(uint* h, const __m128i* input)
{
    assert(h && input);
    __m128i w0, w1, w2, w3;
    v4si prep0, prep1, prep2;
    uint a, b, c, d, e, t;

    a = h[0];
    b = h[1];
    c = h[2];
    d = h[3];
    e = h[4];

    w0 = load(&input[0]);
    prep00_15(prep0, w0);
    w1 = load(&input[1]);
    prep00_15(prep1, w1);
    w2 = load(&input[2]);
    prep00_15(prep2, w2);
    w3 = load(&input[3]);

    step(00_19, a, b, c, d, e, t, prep0.u32[0]);
    step(00_19, t, a, b, c, d, e, prep0.u32[1]);
    step(00_19, e, t, a, b, c, d, prep0.u32[2]);
    step(00_19, d, e, t, a, b, c, prep0.u32[3]);
    prep00_15(prep0, w3);
    step(00_19, c, d, e, t, a, b, prep1.u32[0]);
    step(00_19, b, c, d, e, t, a, prep1.u32[1]);
    step(00_19, a, b, c, d, e, t, prep1.u32[2]);
    step(00_19, t, a, b, c, d, e, prep1.u32[3]);
    prep(prep1, w0, w1, w2, w3, k00_19);
    step(00_19, e, t, a, b, c, d, prep2.u32[0]);
    step(00_19, d, e, t, a, b, c, prep2.u32[1]);
    step(00_19, c, d, e, t, a, b, prep2.u32[2]);
    step(00_19, b, c, d, e, t, a, prep2.u32[3]);
    prep(prep2, w1, w2, w3, w0, k20_39);
    step(00_19, a, b, c, d, e, t, prep0.u32[0]);
    step(00_19, t, a, b, c, d, e, prep0.u32[1]);
    step(00_19, e, t, a, b, c, d, prep0.u32[2]);
    step(00_19, d, e, t, a, b, c, prep0.u32[3]);
    prep(prep0, w2, w3, w0, w1, k20_39);
    step(00_19, c, d, e, t, a, b, prep1.u32[0]);
    step(00_19, b, c, d, e, t, a, prep1.u32[1]);
    step(00_19, a, b, c, d, e, t, prep1.u32[2]);
    step(00_19, t, a, b, c, d, e, prep1.u32[3]);
    prep(prep1, w3, w0, w1, w2, k20_39);
    step(20_39, e, t, a, b, c, d, prep2.u32[0]);
    step(20_39, d, e, t, a, b, c, prep2.u32[1]);
    step(20_39, c, d, e, t, a, b, prep2.u32[2]);
    step(20_39, b, c, d, e, t, a, prep2.u32[3]);
    prep(prep2, w0, w1, w2, w3, k20_39);
    step(20_39, a, b, c, d, e, t, prep0.u32[0]);
    step(20_39, t, a, b, c, d, e, prep0.u32[1]);
    step(20_39, e, t, a, b, c, d, prep0.u32[2]);
    step(20_39, d, e, t, a, b, c, prep0.u32[3]);
    prep(prep0, w1, w2, w3, w0, k20_39);
    step(20_39, c, d, e, t, a, b, prep1.u32[0]);
    step(20_39, b, c, d, e, t, a, prep1.u32[1]);
    step(20_39, a, b, c, d, e, t, prep1.u32[2]);
    step(20_39, t, a, b, c, d, e, prep1.u32[3]);
    prep(prep1, w2, w3, w0, w1, k40_59);
    step(20_39, e, t, a, b, c, d, prep2.u32[0]);
    step(20_39, d, e, t, a, b, c, prep2.u32[1]);
    step(20_39, c, d, e, t, a, b, prep2.u32[2]);
    step(20_39, b, c, d, e, t, a, prep2.u32[3]);
    prep(prep2, w3, w0, w1, w2, k40_59);
    step(20_39, a, b, c, d, e, t, prep0.u32[0]);
    step(20_39, t, a, b, c, d, e, prep0.u32[1]);
    step(20_39, e, t, a, b, c, d, prep0.u32[2]);
    step(20_39, d, e, t, a, b, c, prep0.u32[3]);
    prep(prep0, w0, w1, w2, w3, k40_59);
    step(40_59, c, d, e, t, a, b, prep1.u32[0]);
    step(40_59, b, c, d, e, t, a, prep1.u32[1]);
    step(40_59, a, b, c, d, e, t, prep1.u32[2]);
    step(40_59, t, a, b, c, d, e, prep1.u32[3]);
    prep(prep1, w1, w2, w3, w0, k40_59);
    step(40_59, e, t, a, b, c, d, prep2.u32[0]);
    step(40_59, d, e, t, a, b, c, prep2.u32[1]);
    step(40_59, c, d, e, t, a, b, prep2.u32[2]);
    step(40_59, b, c, d, e, t, a, prep2.u32[3]);
    prep(prep2, w2, w3, w0, w1, k40_59);
    step(40_59, a, b, c, d, e, t, prep0.u32[0]);
    step(40_59, t, a, b, c, d, e, prep0.u32[1]);
    step(40_59, e, t, a, b, c, d, prep0.u32[2]);
    step(40_59, d, e, t, a, b, c, prep0.u32[3]);
    prep(prep0, w3, w0, w1, w2, k60_79);
    step(40_59, c, d, e, t, a, b, prep1.u32[0]);
    step(40_59, b, c, d, e, t, a, prep1.u32[1]);
    step(40_59, a, b, c, d, e, t, prep1.u32[2]);
    step(40_59, t, a, b, c, d, e, prep1.u32[3]);
    prep(prep1, w0, w1, w2, w3, k60_79);
    step(40_59, e, t, a, b, c, d, prep2.u32[0]);
    step(40_59, d, e, t, a, b, c, prep2.u32[1]);
    step(40_59, c, d, e, t, a, b, prep2.u32[2]);
    step(40_59, b, c, d, e, t, a, prep2.u32[3]);
    prep(prep2, w1, w2, w3, w0, k60_79);
    step(60_79, a, b, c, d, e, t, prep0.u32[0]);
    step(60_79, t, a, b, c, d, e, prep0.u32[1]);
    step(60_79, e, t, a, b, c, d, prep0.u32[2]);
    step(60_79, d, e, t, a, b, c, prep0.u32[3]);
    prep(prep0, w2, w3, w0, w1, k60_79);
    step(60_79, c, d, e, t, a, b, prep1.u32[0]);
    step(60_79, b, c, d, e, t, a, prep1.u32[1]);
    step(60_79, a, b, c, d, e, t, prep1.u32[2]);
    step(60_79, t, a, b, c, d, e, prep1.u32[3]);
    prep(prep1, w3, w0, w1, w2, k60_79);
    step(60_79, e, t, a, b, c, d, prep2.u32[0]);
    step(60_79, d, e, t, a, b, c, prep2.u32[1]);
    step(60_79, c, d, e, t, a, b, prep2.u32[2]);
    step(60_79, b, c, d, e, t, a, prep2.u32[3]);

    step(60_79, a, b, c, d, e, t, prep0.u32[0]);
    step(60_79, t, a, b, c, d, e, prep0.u32[1]);
    step(60_79, e, t, a, b, c, d, prep0.u32[2]);
    step(60_79, d, e, t, a, b, c, prep0.u32[3]);
    step(60_79, c, d, e, t, a, b, prep1.u32[0]);
    step(60_79, b, c, d, e, t, a, prep1.u32[1]);
    step(60_79, a, b, c, d, e, t, prep1.u32[2]);
    step(60_79, t, a, b, c, d, e, prep1.u32[3]);

    h[0] += e;
    h[1] += t;
    h[2] += a;
    h[3] += b;
    h[4] += c;
}

void sha1::update(const byte* src, int len)
{
    assert(src);
    /* setup mod info */
    int numsteps = len / 64, rs1 = 0, rs2 = 0;
    int mod1 = len % 64;
    mod1 < 56 ? rs1 = 56 - mod1 : rs2 = 64 - mod1;
    auto* input = (const __m128i*)src;
    for(int i = 0; i < numsteps; i ++, input += 4)
        update_once(_digest, input);
    assert(!(rs1 && rs2));
    if(rs1) {
        /* padding buff */
        byte buff[64];
        memcpy_s(buff, sizeof(buff), input, mod1);
        buff[mod1] = 0x80;
        if(int sz0 = rs1 - 1)
            memset(buff + mod1 + 1, 0, sz0);
        rev_write(buff + 56, (uint64)len * 8);
        /* padding */
        update_once(_digest, (const __m128i*)buff);
    }
    else if(rs2) {
        /* padding buff */
        byte buff1[64], buff2[64];
        memcpy_s(buff1, sizeof(buff1), input, mod1);
        buff1[mod1] = 0x80;
        if(int sz0 = rs2 - 1)
            memset(buff1 + mod1 + 1, 0, sz0);
        memset(buff2, 0, 56);
        rev_write(buff2 + 56, (uint64)len * 8);
        /* padding */
        update_once(_digest, (const __m128i*)buff1);
        update_once(_digest, (const __m128i*)buff2);
    }
}

static inline gchar hexconv(uint x)
{
    if(x >= 0 && x < 10)
        return x + _t('0');
    else if(x >= 10 && x < 16)
        return x - 10 + _t('A');
    return 0;
}

void sha1::get_string(string& str) const
{
    str.clear();
    for(int i = 0; i < _countof(_digest); i ++) {
        auto d = _digest[i];
        d = rev_endian(d);
        auto* r = (const byte*)&d;
        str.push_back(hexconv(r[0]>>4));
        str.push_back(hexconv(r[0]&0xf));
        str.push_back(hexconv(r[1]>>4));
        str.push_back(hexconv(r[1]&0xf));
        str.push_back(hexconv(r[2]>>4));
        str.push_back(hexconv(r[2]&0xf));
        str.push_back(hexconv(r[3]>>4));
        str.push_back(hexconv(r[3]&0xf));
    }
}

__gslib_end__
