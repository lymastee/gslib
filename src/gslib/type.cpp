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

#include <algorithm>
#include <gslib/std.h>
#include <gslib/type.h>

__gslib_begin__

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
inline bool _is_rect_contained(const _rect& rc1, const _rect& rc2)
{
    assert(rc1.left <= rc1.right && rc1.top <= rc1.bottom &&
        rc2.left <= rc2.right && rc2.top <= rc2.bottom
        );
    return rc1.left <= rc2.left && rc1.top <= rc2.top && rc1.right >= rc2.right && rc1.bottom >= rc2.bottom;
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

/* point-rect position relationship:
 *     TL   |       T       |   TR      
 *  --------+---------------+--------   
 *     L    |       C       |   R       
 *  --------+---------------+--------   
 *     BL   |       B       |   BR      
 */
enum prp_relation_type
{
    prpr_TL,
    prpr_L,
    prpr_BL,
    prpr_T,
    prpr_C,
    prpr_B,
    prpr_TR,
    prpr_R,
    prpr_BR,
};

template<class _point, class _rect>
static prp_relation_type query_prp_relation(const _point& pt, const _rect& rc)
{
    if(pt.x < rc.left) {
        if(pt.y < rc.top)
            return prpr_TL;
        else if(pt.y > rc.bottom)
            return prpr_BL;
        return prpr_L;
    }
    else if(pt.x > rc.right) {
        if(pt.y < rc.top)
            return prpr_TR;
        else if(pt.y > rc.bottom)
            return prpr_BR;
        return prpr_R;
    }
    else {
        if(pt.y < rc.top)
            return prpr_T;
        else if(pt.y > rc.bottom)
            return prpr_B;
        return prpr_C;
    }
}

static inline const pointf& convert_pt(const pointf& p) { return p; }
static inline pointf convert_pt(const point& p) { return pointf((float)p.x, (float)p.y); }

static inline bool is_point_upside(const pointf& p, const pointf& p1, const pointf& p2)
{
    /* This file cannot use ariel.utility, and to ensure precision, we use double here. */
    double dx = (double)(p2.x - p1.x);
    double dy = (double)(p2.y - p1.y);
    double a = dy;
    double b = -dx;
    double c = dx * (double)p1.y - dy * (double)p1.x;
    return a * (double)p.x + b * (double)p.y + c >= 0.0;
}

template<class _point, class _rect>
static bool _is_line_rect_overlapped(const _point& p1, const _point& p2, const _rect& rc)
{
    auto rel1 = query_prp_relation(p1, rc);
    if(rel1 == prpr_C)
        return true;
    auto rel2 = query_prp_relation(p2, rc);
    if(rel2 == prpr_C)
        return true;
    switch(rel1)
    {
    case prpr_TL:
        switch(rel2)
        {
        case prpr_B:
            return is_point_upside(convert_pt(rc.bottom_left()), convert_pt(p2), convert_pt(p1));
        case prpr_R:
            return is_point_upside(convert_pt(rc.top_right()), convert_pt(p1), convert_pt(p2));
        case prpr_BR:
            return true;
        }
        break;
    case prpr_L:
        switch(rel2)
        {
        case prpr_T:
        case prpr_TR:
            return is_point_upside(convert_pt(rc.top_left()), convert_pt(p1), convert_pt(p2));
        case prpr_B:
        case prpr_BR:
            return is_point_upside(convert_pt(rc.bottom_left()), convert_pt(p2), convert_pt(p1));
        case prpr_R:
            return true;
        }
        break;
    case prpr_BL:
        switch(rel2)
        {
        case prpr_T:
            return is_point_upside(convert_pt(rc.top_left()), convert_pt(p1), convert_pt(p2));
        case prpr_R:
            return is_point_upside(convert_pt(rc.bottom_right()), convert_pt(p2), convert_pt(p1));
        case prpr_TR:
            return true;
        }
        break;
    case prpr_T:
        switch(rel2)
        {
        case prpr_L:
        case prpr_BL:
            return is_point_upside(convert_pt(rc.top_left()), convert_pt(p2), convert_pt(p1));
        case prpr_R:
        case prpr_BR:
            return is_point_upside(convert_pt(rc.top_right()), convert_pt(p1), convert_pt(p2));
        case prpr_B:
            return true;
        }
        break;
    case prpr_B:
        switch(rel2)
        {
        case prpr_L:
        case prpr_TL:
            return is_point_upside(convert_pt(rc.bottom_left()), convert_pt(p1), convert_pt(p2));
        case prpr_R:
        case prpr_TR:
            return is_point_upside(convert_pt(rc.bottom_right()), convert_pt(p2), convert_pt(p1));
        case prpr_T:
            return true;
        }
        break;
    case prpr_TR:
        switch(rel2)
        {
        case prpr_L:
            return is_point_upside(convert_pt(rc.top_left()), convert_pt(p2), convert_pt(p1));
        case prpr_B:
            return is_point_upside(convert_pt(rc.bottom_right()), convert_pt(p1), convert_pt(p2));
        case prpr_BL:
            return true;
        }
        break;
    case prpr_R:
        switch(rel2)
        {
        case prpr_T:
        case prpr_TL:
            return is_point_upside(convert_pt(rc.top_right()), convert_pt(p2), convert_pt(p1));
        case prpr_B:
        case prpr_BL:
            return is_point_upside(convert_pt(rc.bottom_right()), convert_pt(p1), convert_pt(p2));
        case prpr_L:
            return true;
        }
        break;
    case prpr_BR:
        switch(rel2)
        {
        case prpr_L:
            return is_point_upside(convert_pt(rc.bottom_left()), convert_pt(p1), convert_pt(p2));
        case prpr_T:
            return is_point_upside(convert_pt(rc.top_right()), convert_pt(p2), convert_pt(p1));
        case prpr_TL:
            return true;
        }
        break;
    }
    return false;
}

bool intersect_rect(rect& rc, const rect& rc1, const rect& rc2) { return _intersect_rect(rc, rc1, rc2); }
bool intersect_rect(rectf& rc, const rectf& rc1, const rectf& rc2) { return _intersect_rect(rc, rc1, rc2); }
bool is_rect_intersected(const rect& rc1, const rect& rc2) { return _is_rect_intersected(rc1, rc2); }
bool is_rect_intersected(const rectf& rc1, const rectf& rc2) { return _is_rect_intersected(rc1, rc2); }
bool is_rect_contained(const rect& rc1, const rect& rc2) { return _is_rect_contained(rc1, rc2); }
bool is_rect_contained(const rectf& rc1, const rectf& rc2) { return _is_rect_contained(rc1, rc2); }
void union_rect(rect& rc, const rect& rc1, const rect& rc2) { _union_rect(rc, rc1, rc2); }
void union_rect(rectf& rc, const rectf& rc1, const rectf& rc2) { _union_rect(rc, rc1, rc2); }
bool substract_rect(rect& rc, const rect& rc1, const rect& rc2) { return _substract_rect(rc, rc1, rc2); }
bool substract_rect(rectf& rc, const rectf& rc1, const rectf& rc2) { return _substract_rect(rc, rc1, rc2); }
bool is_line_rect_overlapped(const point& p1, const point& p2, const rect& rc) { return _is_line_rect_overlapped(p1, p2, rc); }
bool is_line_rect_overlapped(const pointf& p1, const pointf& p2, const rectf& rc) { return _is_line_rect_overlapped(p1, p2, rc); }

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

__gslib_end__
