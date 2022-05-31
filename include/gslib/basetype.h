/*
 * Copyright (c) 2016-2021 lymastee, All rights reserved.
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

#pragma once

#ifndef base_type_2aa01495_7feb_4c5e_ae38_9c775c29c9f1_h
#define base_type_2aa01495_7feb_4c5e_ae38_9c775c29c9f1_h

#include <assert.h>
#include <stdint.h>
#include <gslib/config.h>

__gslib_begin__

#if defined(_GS_X86) || defined(_GS_X64)

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef wchar_t wchar;
typedef float real32;
typedef double real;
typedef uint8_t byte;
typedef uint16_t word;
typedef uint32_t dword;
typedef uint64_t qword;
typedef uint32_t uint;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
typedef void* addrptr;

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
