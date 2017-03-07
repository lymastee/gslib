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

#include <algorithm>
#include <gslib/std.h>
#include <pink/type.h>

__pink_begin__

template<class _rect>
inline bool _intersect_rect(_rect& rc, const _rect& rc1, const _rect& rc2)
{
    rc.left = gs_max(rc1.left, rc2.left);
    rc.top = gs_max(rc1.top, rc2.top);
    rc.right = gs_min(rc1.right, rc2.right);
    rc.bottom = gs_min(rc1.bottom, rc2.bottom);
    return (rc.left <= rc.right) && (rc.top <= rc.bottom);
}

template<class _rect>
inline bool _is_rect_intersected(const _rect& rc1, const _rect& rc2)
{
    auto max_left = gs_max(rc1.left, rc2.left);
    auto min_right = gs_min(rc1.right, rc2.right);
    if(max_left > min_right)
        return false;
    auto max_top = gs_max(rc1.top, rc2.top);
    auto min_bottom = gs_min(rc1.bottom, rc2.bottom);
    return max_top <= min_bottom;
}

template<class _rect>
inline void _union_rect(_rect& rc, const _rect& rc1, const _rect& rc2)
{
    rc.left = gs_min(rc1.left, rc2.left);
    rc.right = gs_max(rc1.right, rc2.right);
    rc.top = gs_min(rc1.top, rc2.top);
    rc.bottom = gs_max(rc1.bottom, rc2.bottom);
}

enum coincide_state
{
    coincide_none,
    coincide_top_left,
    coincide_top_right,
    coincide_bottom_left,
    coincide_bottom_right,
    coincide_top,
    coincide_bottom,
    coincide_left,
    coincide_right,
};

static coincide_state query_coincide_state(const rect& rc1, const rect& rc2)
{
    if(rc1.top == rc2.top) {
        if(rc1.left == rc2.left)
            return coincide_top_left;
        else if(rc1.right == rc2.right)
            return coincide_top_right;
        return coincide_top;
    }
    if(rc1.bottom == rc2.bottom) {
        if(rc1.left == rc2.left)
            return coincide_bottom_left;
        else if(rc1.right == rc2.right)
            return coincide_bottom_right;
        return coincide_bottom;
    }
    if(rc1.left == rc2.left)
        return coincide_left;
    else if(rc1.right == rc2.right)
        return coincide_right;
    return coincide_none;
}

static coincide_state query_coincide_state(const rectf& rc1, const rectf& rc2)
{
    const float tol = 1e-5f;
    if(abs(rc1.top - rc2.top) < tol) {
        if(abs(rc1.left - rc2.left) < tol)
            return coincide_top_left;
        else if(abs(rc1.right - rc2.right) < tol)
            return coincide_top_right;
        return coincide_top;
    }
    if(abs(rc1.bottom - rc2.bottom) < tol) {
        if(abs(rc1.left - rc2.left) < tol)
            return coincide_bottom_left;
        else if(abs(rc1.right - rc2.right) < tol)
            return coincide_bottom_right;
        return coincide_bottom;
    }
    if(abs(rc1.left - rc2.left) < tol)
        return coincide_left;
    else if(abs(rc1.right - rc2.right) < tol)
        return coincide_right;
    return coincide_none;
}

template<class _rect>
bool _substract_rect(_rect& rc, const _rect& rc1, const _rect& rc2)
{
    _rect ints;
    if(!intersect_rect(ints, rc1, rc2)) {
        rc = rc1;
        return true;
    }
    switch(query_coincide_state(ints, rc1))
    {
    case coincide_top:
        rc.set_ltrb(rc1.left, ints.bottom, rc1.right, rc1.bottom);
        return true;
    case coincide_bottom:
        rc.set_ltrb(rc1.left, rc1.top, rc1.right, ints.top);
        return true;
    case coincide_left:
        rc.set_ltrb(ints.right, rc1.top, rc1.right, rc1.bottom);
        return true;
    case coincide_right:
        rc.set_ltrb(rc1.left, rc1.top, ints.left, rc1.bottom);
        return true;
    case coincide_top_left:
        rc.set_ltrb(ints.right, ints.bottom, rc1.right, rc1.bottom);
        return true;
    case coincide_top_right:
        rc.set_ltrb(rc1.left, ints.bottom, ints.left, rc1.bottom);
        return true;
    case coincide_bottom_left:
        rc.set_ltrb(ints.right, rc1.top, rc1.right, ints.top);
        return true;
    case coincide_bottom_right:
        rc.set_ltrb(rc1.left, rc1.top, ints.left, ints.top);
        return true;
    default:
        return false;
    }
}

bool intersect_rect(rect& rc, const rect& rc1, const rect& rc2) { return _intersect_rect(rc, rc1, rc2); }
bool intersect_rect(rectf& rc, const rectf& rc1, const rectf& rc2) { return _intersect_rect(rc, rc1, rc2); }
bool is_rect_intersected(const rect& rc1, const rect& rc2) { return _is_rect_intersected(rc1, rc2); }
bool is_rect_intersected(const rectf& rc1, const rectf& rc2) { return _is_rect_intersected(rc1, rc2); }
void union_rect(rect& rc, const rect& rc1, const rect& rc2) { _union_rect(rc, rc1, rc2); }
void union_rect(rectf& rc, const rectf& rc1, const rectf& rc2) { _union_rect(rc, rc1, rc2); }
bool substract_rect(rect& rc, const rect& rc1, const rect& rc2) { return _substract_rect(rc, rc1, rc2); }
bool substract_rect(rectf& rc, const rectf& rc1, const rectf& rc2) { return _substract_rect(rc, rc1, rc2); }

void rect::deflate(int u, int v)
{
    int w = width(), h = height();
    w >>= 1;
    h >>= 1;
    if(u > w) u = w;
    if(v > h) v = h;
    left += u;
    right -= u;
    top += v;
    bottom -= v;
}

void rect::move_to(int x, int y)
{
    right = x + width();
    bottom = y + height();
    left = x;
    top = y;
}

void rectf::deflate(float u, float v)
{
    float w = width(), h = height();
    w /= 2;
    h /= 2;
    if(u > w) u = w;
    if(v > h) v = h;
    left += u;
    right -= u;
    top += v;
    bottom -= v;
}

void rectf::move_to(float x, float y)
{
    right = x + width();
    bottom = y + height();
    left = x;
    top = y;
}

__pink_end__
