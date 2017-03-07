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

#ifndef config_c6d6cb9c_7057_45d5_b554_535425b2892d_h
#define config_c6d6cb9c_7057_45d5_b554_535425b2892d_h

#include <limits.h>
#include <stdarg.h>

#if defined(_WIN32) && !defined(_WIN64)
#define _GS_X86
#endif

#ifdef _GS_X86

/* same size: pointer, int, x86 platform */
#pragma warning(disable: 4311)
#pragma warning(disable: 4312)
#define ptrsize ((int)sizeof(dword))
#endif

#ifndef _countof
#define _countof(a) (sizeof(a)/sizeof(a[0]))
#endif

#ifndef _offsetof
#define _offsetof(s, m) (int)(&((s*)0)->m)
#endif

#ifdef _UNICODE

#ifndef _cststrlen
#define _cststrlen(s) (sizeof(s)/2-1)
#endif

#else

#ifndef _cststrlen
#define _cststrlen(s) (sizeof(s)-1)
#endif

#endif

#ifndef _cststr
#define _cststr(s)  s, (_countof(s) - 1)
#endif

#define __gs_novtable       __declspec(novtable)
#define __gs_naked          __declspec(naked)
#define __gs_selany         __declspec(selectany)

#ifdef __cplusplus

#define __gslib_begin__     namespace gs {
#define __gslib_end__       };

#define switch_namespace(from, to) \
    __##from##_end__ \
    __##to##_begin__

#endif

//#define _GS_THREADSAFE

#undef declare_mask
#define declare_mask(msk, shl)  msk = (1<<shl)

#undef verify

#if defined (DEBUG) || defined (_DEBUG)
#define verify(exp)         assert(exp)
#else
#define verify(exp)         bool(exp)
#endif

#ifdef _UNICODE
#define _t(s)               L##s
#else
#define _t(s)               s
#endif

#ifdef __cplusplus

#define define_select_type(name) \
    template<unsigned _condition> struct select_##name {};

#define install_select_type(cond, name, selt) \
    template<> struct select_##name<cond> { typedef selt seltype; };

#define config_select_type(env, name) \
    typedef select_##name<env>::seltype name;

#define static_cast_as(type, name, var) \
    type name = static_cast<type>(var);

#define reinterpret_cast_as(type, name, var) \
    type name = reinterpret_cast<type>(var);

#define max_sizeof(type1, type2) \
    (sizeof(type1) > sizeof(type2) ? sizeof(type1) : sizeof(type2))

#define min_sizeof(type1, type2) \
    (sizeof(type1) < sizeof(type2) ? sizeof(type1) : sizeof(type2))

#define max_sizeof3(type1, type2, type3) \
    (sizeof(type1) > sizeof(type2) ? (sizeof(type1) > sizeof(type3) ? sizeof(type1) : sizeof(type3)) : \
    (sizeof(type3) > sizeof(type2) ? sizeof(type3) : sizeof(type2)) \
    )

#define min_sizeof3(type1, type2, type3) \
    (sizeof(type1) < sizeof(type2) ? (sizeof(type1) < sizeof(type3) ? sizeof(type1) : sizeof(type3)) : \
    (sizeof(type3) < sizeof(type2) ? sizeof(type3) : sizeof(type2)) \
    )

#endif

#endif
