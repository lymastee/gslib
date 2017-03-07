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

#ifndef type_43211674_a64f_450a_b8cb_67bab6092bb5_h
#define type_43211674_a64f_450a_b8cb_67bab6092bb5_h

#include <pink/config.h>
#include <gslib/type.h>
#include <gslib/string.h>
#include <gslib/math.h>

__pink_begin__

struct color;
struct pixel;
void convert_color_to_pixel(pixel& p, const color& cr);

struct pixel
{
    byte    blue, green, red;

public:
    pixel() { red = 0; green = 0; blue = 0; }
    pixel(int cr) { from_xrgb(cr); }
    pixel(int r, int g, int b) { red = r; green = g; blue = b; }
    dword to_rgb() const { return (dword)(red << 24) | (green << 16) | (blue << 8); }
    dword to_bgr() const { return (dword) red | (green << 8) | (blue << 16); }
    void from_color(const color& cr) { convert_color_to_pixel(*this, cr); }
    void from_rgbx(int cr) { from_xrgb(cr >>= 8); }
    void from_xrgb(int cr) { blue = cr; green = (cr >>= 8); red = (cr >>= 8); }
    const pixel& blend(const pixel& c1, const pixel& c2, float f) { return blend_color(*this, c1, c2, f); }
    const pixel& blend(const pixel& c1, const pixel& c2, float f1, float f2) { return blend_color(*this, c1, c2, f1, f2); }
    bool operator !=(const pixel& that) const { return blue != that.blue || green != that.green || red != that.red; }
    bool operator ==(const pixel& that) const { return blue == that.blue && green == that.green && red == that.red; }

    static inline const pixel& blend_color(pixel& c, const pixel& c1, const pixel& c2, float f1, float f2)
    {
        c.red = round(f1 * c1.red + f2 * c2.red);
        c.green = round(f1 * c1.green + f2 * c2.green);
        c.blue = round(f1 * c1.blue + f2 * c2.blue);
        return c;
    }
    static inline const pixel& blend_color(pixel& c, const pixel& c1, const pixel& c2, float f) { return blend_color(c, c1, c2, f, 1.f-f); }
};

struct color
{
    uint    red: 8, green: 8, blue: 8, alpha: 8;

public:
    color() { data() = 0; }
    color(int r, int g, int b) { set_color(r, g, b); }
    color(int r, int g, int b, int a) { set_color(r, g, b, a); }
    uint& data() { return *(uint*)this; }
    uint data() const { return *(uint*)this; }
    void set_color(int r, int g, int b) { red = r, green = g, blue = b, alpha = 255; }
    void set_color(int r, int g, int b, int a) { red = r, green = g, blue = b, alpha = a; }
    void from_pixel(const pixel& p) { set_color(p.red, p.green, p.blue); }
    bool operator !=(const color& cr) const { return blue != cr.blue || green != cr.green || red != cr.red || alpha != cr.alpha; }
    bool operator ==(const color& cr) const { return blue == cr.blue && green == cr.green && red == cr.red && alpha == cr.alpha; }
};

inline void convert_color_to_pixel(pixel& p, const color& cr)
{
    p.red = cr.red;
    p.green = cr.green;
    p.blue = cr.blue;
}

struct font
{
    enum
    {
        declare_mask(ftm_italic,    0),
        declare_mask(ftm_underline, 1),
        declare_mask(ftm_strikeout, 2),
    };

public:
    string  name;
    int     height;
    int     width;
    int     escape;
    int     orient;
    int     weight; /* 0-9 */
    uint    mask;

public:
    font()
    {
        height = 0;
        width = 0;
        escape = 0;
        orient = 0;
        weight = 0;
        mask = 0;
    }
    font(const font& that)
    {
        name    = that.name;
        height  = that.height;
        width   = that.width;
        escape  = that.escape;
        orient  = that.orient;
        weight  = that.weight;
        mask    = that.mask;
    }
    font(const gchar* n, int size)
    {
        name.assign(n);
        height = size;
        width = 0;
        escape = 0;
        orient = 0;
        weight = 0;
        mask = 0;
    }
    font& operator = (const font& that)
    {
        name    = that.name;
        height  = that.height;
        width   = that.width;
        escape  = that.escape;
        orient  = that.orient;
        weight  = that.weight;
        mask    = that.mask;
        return *this;
    }
    bool operator == (const font& that) const
    {
        if(name != that.name)
            return false;
        if(height != that.height)
            return false;
        if(width != that.width)
            return false;
        if(escape != that.escape)
            return false;
        if(orient != that.orient)
            return false;
        if(weight != that.weight)
            return false;
        if(mask != that.mask)
            return false;
        return true;
    }
    bool operator != (const font& that) const
    {
        if(name != that.name)
            return true;
        if(height != that.height)
            return true;
        if(width != that.width)
            return true;
        if(escape != that.escape)
            return true;
        if(orient != that.orient)
            return true;
        if(weight != that.weight)
            return true;
        if(mask != that.mask)
            return true;
        return false;
    }
};

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

__pink_end__

#endif
