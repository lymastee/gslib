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

#ifndef type_f33c5e03_70f2_4879_9504_332323832a40_h
#define type_f33c5e03_70f2_4879_9504_332323832a40_h

#include <ariel/config.h>
#include <gslib/type.h>
#include <gslib/string.h>
#include <gslib/math.h>
#include <gslib/std.h>

__ariel_begin__

struct ariel_export color
{
    union
    {
        struct { byte red, green, blue, alpha; };
        uint _data;
    };

public:
    color() { data() = 0; }
    color(int r, int g, int b) { set_color(r, g, b); }
    color(int r, int g, int b, int a) { set_color(r, g, b, a); }
    uint& data() { return _data; }
    uint data() const { return _data; }
    void set_color(int r, int g, int b) { red = r, green = g, blue = b, alpha = 255; }
    void set_color(int r, int g, int b, int a) { red = r, green = g, blue = b, alpha = a; }
    bool operator !=(const color& cr) const { return blue != cr.blue || green != cr.green || red != cr.red || alpha != cr.alpha; }
    bool operator ==(const color& cr) const { return blue == cr.blue && green == cr.green && red == cr.red && alpha == cr.alpha; }
    color& lerp(const color& c1, const color& c2, float s)
    {
        assert(s >= 0.f && s <= 1.f);
        float t = 1.f - s;
        red = (int)(s * c1.red + t * c2.red);
        green = (int)(s * c1.green + t * c2.green);
        blue = (int)(s * c1.blue + t * c2.blue);
        alpha = (int)(s * c1.alpha + t * c2.alpha);
        return *this;
    }
};

struct ariel_export font
{
    enum
    {
        declare_mask(ftm_italic,    0),
        declare_mask(ftm_underline, 1),
        declare_mask(ftm_strikeout, 2),
    };

public:
    string      name;
    int         size;
    int         escape;
    int         orient;
    int         weight; /* 0-9 */
    uint        mask;

private:
    friend class fsys_win32;
    friend class fsys_dwrite;
    mutable uint sysfont;

public:
    font()
    {
        size = 0;
        escape = 0;
        orient = 0;
        weight = 3;
        mask = 0;
        sysfont = 0;
    }
    font(const font& that)
    {
        name    = that.name;
        size    = that.size;
        escape  = that.escape;
        orient  = that.orient;
        weight  = that.weight;
        mask    = that.mask;
        sysfont = that.sysfont;
    }
    font(const gchar* n, int sz)
    {
        name.assign(n);
        size = sz;
        escape = 0;
        orient = 0;
        weight = 3;
        mask = 0;
        sysfont = 0;
    }
    font& operator = (const font& that)
    {
        name    = that.name;
        size  = that.size;
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
        if(size != that.size)
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
        if(size != that.size)
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
    size_t hash_value() const
    {
        hasher h;
        h.add_bytes((const byte*)name.c_str(), name.length() * sizeof(gchar));
        h.add_bytes((const byte*)&size, sizeof(size));
        h.add_bytes((const byte*)&escape, sizeof(escape));
        h.add_bytes((const byte*)&orient, sizeof(orient));
        h.add_bytes((const byte*)&weight, sizeof(weight));
        return h.add_bytes((const byte*)&mask, sizeof(mask));
    }
};

struct ariel_export viewport
{
    float           left;
    float           top;
    float           width;
    float           height;
    float           min_depth;
    float           max_depth;
};

struct ariel_export axis_aligned_bound_box
{
    float           left = FLT_MAX;
    float           right = -FLT_MAX;
    float           top = FLT_MAX;
    float           bottom = -FLT_MAX;
    float           front = FLT_MAX;
    float           back = -FLT_MAX;

public:
    void reset()
    {
        left = top = front = FLT_MAX;
        right = bottom = back = -FLT_MAX;
    }
    float width() const { return right - left; }
    float height() const { return bottom - top; }
    float depth() const { return back - front; }
};

struct ariel_export origin_bound_sphere
{
    float           radius = 0.f;
};

struct ariel_export bound_sphere
{
    vec3            origin;
    float           radius = 0.f;
};

enum res_type
{
    res_mesh,
};

class __gs_novtable ariel_export res_node abstract
{
public:
    virtual ~res_node() {}
    virtual res_type get_type() const = 0;
    virtual const string& get_name() const { return _name; }
    virtual bool has_name() const { return !_name.empty(); }

protected:
    string          _name;

public:
    void set_name(const string& name) { _name = name; }
};

__ariel_end__

namespace std {

template<>
class hash<gs::ariel::font>
#if defined(_MSC_VER) && (_MSC_VER < 1914)
    : public unary_function<gs::ariel::font, size_t>
#endif
{
public:
    size_t operator()(const gs::ariel::font& ft) const { return ft.hash_value(); }
};

};

#endif
