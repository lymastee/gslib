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

#ifndef type_3f1a28e2_0da6_44b6_84db_29542f5a65c0_h
#define type_3f1a28e2_0da6_44b6_84db_29542f5a65c0_h

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

#endif  /* end of __gs_x86__ */

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

#include <gslib/math.h>

__gslib_begin__

template<class _ty> struct protopt {};
template<> struct protopt<int> { int x, y; };
template<>
struct protopt<float>: public vec2
{
    protopt() {}
    protopt(const vec2& v): vec2(v) {}
    protopt(float x, float y): vec2(x, y) {}
};

template<class _ty>
struct point_t:
    public protopt<_ty>
{
    typedef _ty type;
    typedef protopt<_ty> proto;
    typedef point_t<_ty> myref;

public:
    point_t() { x = 0; y = 0; }
    point_t(const proto& p): proto(p) {}
    point_t(type a, type b) { x = a; y = b; }
    void offset(type u, type v) { x += u; y += v; }
    void set_point(type a, type b) { x = a; y = b; }
    bool operator == (const myref& that) const { return x == that.x && y == that.y; }
    bool operator != (const myref& that) const { return x != that.x || y != that.y; }
};

typedef point_t<int> point;
typedef point_t<float> pointf;

template<class _ty>
struct rect_t
{
    typedef _ty type;
    typedef rect_t<_ty> myref;
    typedef point_t<_ty> point;

public:
    type    left, top, right, bottom;

public:
    rect_t()
    {
        left = 0;
        top = 0;
        right = 0;
        bottom = 0;
    }
    rect_t(type l, type t, type w, type h) { set_rect(l, t, w, h); }
    type width() const { return right - left; }
    type height() const { return bottom - top; }
    void set_rect(type l, type t, type w, type h)
    {
        left = l;
        top = t;
        right = l + w;
        bottom = t + h;
    }
    void set_ltrb(type l, type t, type r, type b)
    {
        left = l;
        top = t;
        right = r;
        bottom = b;
    }
    bool in_rect(const point& pt) const { return pt.x >= left && pt.x < right && pt.y >= top && pt.y < bottom; }
    void offset(type x, type y) { left += x; right += x; top += y; bottom += y; }
    void deflate(type u, type v);
    void move_to(const point& pt) { move_to(pt.x, pt.y); }
    void move_to(type x, type y);
    bool operator == (const myref& that) const { return left == that.left && right == that.right && top == that.top && bottom == that.bottom; }
    bool operator != (const myref& that) const { return left != that.left || right != that.right || top != that.top || bottom != that.bottom; }
    type area() const { return width() * height(); }
    point center() const
    {
        point c;
        c.x = (left + right) / 2;
        c.y = (top + bottom) / 2;
        return c;
    }
};

typedef rect_t<int> rect;
typedef rect_t<float> rectf;

extern bool intersect_rect(rect& rc, const rect& rc1, const rect& rc2);
extern bool is_rect_intersected(const rect& rc1, const rect& rc2);
extern void union_rect(rect& rc, const rect& rc1, const rect& rc2);
extern bool substract_rect(rect& rc, const rect& rc1, const rect& rc2);
extern bool intersect_rect(rectf& rc, const rectf& rc1, const rectf& rc2);
extern bool is_rect_intersected(const rectf& rc1, const rectf& rc2);
extern void union_rect(rectf& rc, const rectf& rc1, const rectf& rc2);
extern bool substract_rect(rectf& rc, const rectf& rc1, const rectf& rc2);

__gslib_end__

#endif
