/*
 * Copyright (c) 2016-2018 lymastee, All rights reserved.
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

#ifndef base_type_2aa01495_7feb_4c5e_ae38_9c775c29c9f1_h
#define base_type_2aa01495_7feb_4c5e_ae38_9c775c29c9f1_h

#include <assert.h>
#include <gslib/config.h>

__gslib_begin__

#ifdef _GS_X86

typedef char int8;
typedef short int16;
typedef int int32;
typedef long long int64;
typedef wchar_t wchar;
typedef float real32;
typedef double real;
typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned int dword;
typedef unsigned long long qword;
typedef dword uint;
typedef word uint16;
typedef dword uint32;
typedef qword uint64;

#ifdef _UNICODE
typedef wchar gchar;
#else
typedef char gchar;
#endif

template<class t>
struct typeof { typedef void type; };

/* virtual class ptr convert tools */
template<class ccp, class bcp>
inline int virtual_bias()
{
    static typeof<ccp>::type c;
    static const int bias = (int)&c - (int)static_cast<bcp>(&c);
    return bias;
}

template<class ccp, class bcp>
inline ccp virtual_cast(bcp p)
{
    byte* ptr = (byte*)p;
    ptr += virtual_bias<ccp, bcp>();
    return reinterpret_cast<ccp>(ptr);
}

#endif  /* end of _GS_X86 */

template<class _ty>
inline _ty gs_min(_ty a, _ty b) { return a < b ? a : b; }
template<class _ty>
inline _ty gs_max(_ty a, _ty b) { return a > b ? a : b; }
template<class _ty>
inline _ty gs_clamp(_ty v, _ty a, _ty b)
{
    assert(a <= b);
    return gs_min(b, gs_max(a, v));
}

template<class _ty>
inline void gs_swap(_ty& a, _ty& b)
{
    auto t = a;
    a = b;
    b = t;
}

__gslib_end__

#endif
