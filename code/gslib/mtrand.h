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

#ifndef mtrand_79b71691_32bd_4bf2_9da9_d91e5b8a19b1_h
#define mtrand_79b71691_32bd_4bf2_9da9_d91e5b8a19b1_h

#include <gslib/type.h>

/*
 * This was a rand tool use mersenne twist, which was behaved much better than the default rand.
 * Original version was written by Geoff Kuenning, compiled by gcc.
 * We added some neccessary headers to support standard C99.
 * Some changes have been made to make it possibly to compile by visual c++.
 * Thanks Geoff Kuenning, and Takuji Nishimura, and everyone who participate in that distribution. 
 * Best wishes.
 */

#undef MT_GENERATE_CODE_IN_HEADER
#define MT_GENERATE_CODE_IN_HEADER 1
#include <mtrand/mtwist.h>
#undef MT_GENERATE_CODE_IN_HEADER

__gslib_begin__

inline void mtsrand(uint seed) { mt_seed32new(seed); }
inline uint mtseed() { return mt_seed(); }
inline uint mtgoodseed() { return mt_goodseed(); }
inline void mtbestseed() { mt_bestseed(); }
inline uint mtrand() { return mt_lrand(); }
inline uint64 mtrand64() { return mt_llrand(); }
inline real32 mtrandf() { return (real32)mt_drand(); }
inline real mtrandd() { return mt_ldrand(); }

__gslib_end__

#endif
