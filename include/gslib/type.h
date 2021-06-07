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

#ifndef type_3f1a28e2_0da6_44b6_84db_29542f5a65c0_h
#define type_3f1a28e2_0da6_44b6_84db_29542f5a65c0_h

#include <utility>
#include <gslib/basetype.h>
#include <gslib/math.h>

__gslib_begin__

template<class _ty, class _protopt>
struct point_t:
    public _protopt
{
    typedef _ty type;
    typedef _protopt proto;
    typedef point_t<_ty, _protopt> myref;

public:
    point_t() { this->x = 0; this->y = 0; }
    point_t(const proto& p): proto(p) {}
    point_t(type a, type b) { this->x = a; this->y = b; }
    void offset(type u, type v) { this->x += u; this->y += v; }
    void offset(const proto& p) { this->x += p.x; this->y += p.y; }
    void set_point(type a, type b) { this->x = a; this->y = b; }
    bool operator == (const myref& that) const { return this->x == that.x && this->y == that.y; }
    bool operator != (const myref& that) const { return this->x != that.x || this->y != that.y; }
};

struct vec2i { int x, y; };
typedef point_t<int, vec2i> point;
typedef point_t<float, vec2> pointf;

template<class _ty, class _ptcls>
struct rect_t
{
    typedef _ty type;
    typedef rect_t<_ty, _ptcls> myref;
    typedef _ptcls point;

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
    void set_by_pts(const point& p1, const point& p2)
    {
        left = gs_min(p1.x, p2.x);
        top = gs_min(p1.y, p2.y);
        right = gs_max(p1.x, p2.x);
        bottom = gs_max(p1.y, p2.y);
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
    point top_left() const { return point(left, top); }
    point top_right() const { return point(right, top); }
    point bottom_left() const { return point(left, bottom); }
    point bottom_right() const { return point(right, bottom); }
};

typedef rect_t<int, point> rect;
typedef rect_t<float, pointf> rectf;

inline rectf to_rectf(const rect& rc)
{
    return std::move(rectf((float)rc.left, (float)rc.top, (float)rc.width(), (float)rc.height()));
}

inline rect to_aligned_rect(const rectf& rc)
{
    return std::move(rect(round(rc.left), round(rc.top), round(rc.width()), round(rc.height())));
}

extern bool intersect_rect(rect& rc, const rect& rc1, const rect& rc2);
extern bool is_rect_intersected(const rect& rc1, const rect& rc2);
extern void union_rect(rect& rc, const rect& rc1, const rect& rc2);
extern bool substract_rect(rect& rc, const rect& rc1, const rect& rc2);
extern bool intersect_rect(rectf& rc, const rectf& rc1, const rectf& rc2);
extern bool is_rect_intersected(const rectf& rc1, const rectf& rc2);
extern bool is_rect_contained(const rect& rc1, const rect& rc2);
extern bool is_rect_contained(const rectf& rc1, const rectf& rc2);
extern void union_rect(rectf& rc, const rectf& rc1, const rectf& rc2);
extern bool substract_rect(rectf& rc, const rectf& rc1, const rectf& rc2);
extern bool is_line_rect_overlapped(const point& p1, const point& p2, const rect& rc);
extern bool is_line_rect_overlapped(const pointf& p1, const pointf& p2, const rectf& rc);

__gslib_end__

#endif
