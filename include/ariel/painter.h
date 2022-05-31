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

#ifndef painter_0e5e866a_20d2_4fb1_93c0_1785588121f2_h
#define painter_0e5e866a_20d2_4fb1_93c0_1785588121f2_h

#include <memory>
#include <ariel/sysop.h>
#include <ariel/image.h>
#include <ariel/painterpath.h>

__ariel_begin__

struct painter_data
{
protected:
    virtual ~painter_data() {}
};

struct painter_picture_data:
    public painter_data
{
protected:
    texture2d*          _image;
    rectf               _src_rect;

public:
    painter_picture_data() { _image = nullptr; }
    painter_picture_data(texture2d* p);
    painter_picture_data(texture2d* p, const rectf& rc);
    void set_image(texture2d* p) { _image = p; }
    texture2d* get_image() const { return _image; }
    void set_src_rect(const rectf& rc) { _src_rect = rc; }
    const rectf& get_src_rect() const { return _src_rect; }
};

typedef std::shared_ptr<painter_data> painter_extra_data;

struct painter_brush
{
public:
    enum
    {
        none,
        solid,
        picture,
    };
    typedef painter_extra_data extra_data;

protected:
    uint                _tag;
    color               _color;
    extra_data          _extra;

public:
    painter_brush() { _tag = none; }
    painter_brush(uint tag) { _tag = tag; }
    painter_brush(uint tag, const color& cr) { _tag = tag; _color = cr; }
    void set_tag(uint tag) { _tag = tag; }
    uint get_tag() const { return _tag; }
    void set_color(const color& cr) { _color = cr; }
    const color& get_color() const { return _color; }
    void set_extra(const extra_data& ext) { _extra = ext; }
    const extra_data& get_extra() const { return _extra; }
};

struct painter_pen
{
public:
    enum
    {
        none,
        solid,
        picture,
    };
    typedef painter_extra_data extra_data;

protected:
    uint                _tag;
    color               _color;
    extra_data          _extra;

public:
    painter_pen() { _tag = none; }
    painter_pen(uint tag) { _tag = tag; }
    painter_pen(uint tag, const color& cr) { _tag = tag; _color = cr; }
    void set_tag(uint tag) { _tag = tag; }
    uint get_tag() const { return _tag; }
    void set_color(const color& cr) { _color = cr; }
    const color& get_color() const { return _color; }
    void set_extra(const extra_data& ext) { _extra = ext; }
    const extra_data& get_extra() const { return _extra; }
};

struct painter_context
{
    painter_brush       _brush;
    painter_pen         _pen;
    mat3                _transform;

    painter_context() { _transform.identity(); }
    void set_brush(const painter_brush& brush) { _brush = brush; }
    void set_pen(const painter_pen& pen) { _pen = pen; }
    void set_transform(const mat3& m) { _transform = m; }
    const painter_brush& get_brush() const { return _brush; }
    const painter_pen& get_pen() const { return _pen; }
    const mat3& get_trasnform() const { return _transform; }
};

class painter_path;

class __gs_novtable painter abstract
{
public:
    enum painter_hints
    {
        hint_antialias     = 0x1,
    };
    typedef vector<texture2d*> text_image_cache;
    typedef painter_context context;
    typedef list<context> context_stack;

public:
    virtual ~painter();
    virtual void resize(int w, int h) = 0;
    virtual void draw_path(const painter_path& path) = 0;

public:
    virtual int get_width() const { return _width; }
    virtual int get_height() const { return _height; }
    virtual void set_dirty(dirty_list* dirty) {}
    virtual dirty_list* get_dirty() const { return nullptr; }
    virtual void set_hints(uint hints, bool enable);
    virtual void set_brush(const painter_brush& b) { _context.set_brush(b); }
    virtual void set_pen(const painter_pen& p) { _context.set_pen(p); }
    virtual void set_tranform(const mat3& m) { _context.set_transform(m); }
    virtual void set_font(const font& ft);
    virtual void save();
    virtual void restore();
    virtual bool query_hints(uint hints) const { return (_hints & hints) == hints; }
    virtual context& get_context() { return _context; }
    virtual void get_text_dimension(const gchar* str, int& w, int& h, int len = -1);
    virtual void on_draw_begin();
    virtual void on_draw_end();
    virtual void draw_image(texture2d* img, float x, float y);
    virtual void draw_image(texture2d* img, const rectf& dest, const rectf& src);
    virtual void draw_line(const vec2& p1, const vec2& p2, const color& cr);
    virtual void draw_rect(const rectf& rc, const color& cr);
    virtual void draw_text(const gchar* str, float x, float y, const color& cr, int length = -1);

public:
    virtual void draw_line(const vec2& p1, const vec2& p2)
    {
        if(vec2().sub(p1, p2).length() < 0.001f)
            return;
        painter_path path;
        path.move_to(p1);
        path.line_to(p2);
        draw_path(path);
    }
    virtual void draw_rect(const rectf& rc)
    {
        if(!rc.width() || !rc.height())
            return;
        painter_path path;
        path.move_to(rc.left, rc.top);
        path.line_to(rc.left, rc.bottom);
        path.line_to(rc.right, rc.bottom);
        path.line_to(rc.right, rc.top);
        path.close_path();
        draw_path(path);
    }
    virtual void draw_quad(const vec2& p1, const vec2& p2, const vec2& p3)
    {
        painter_path path;
        path.move_to(p1);
        path.quad_to(p2, p3);
        draw_path(path);
    }
    virtual void draw_cubic(const vec2& p1, const vec2& p2, const vec2& p3, const vec2& p4)
    {
        painter_path path;
        path.move_to(p1);
        path.cubic_to(p2, p3, p4);
        draw_path(path);
    }
    virtual void draw_arc(const vec2& p1, const vec2& p2, float r, bool inv = false)
    {
        painter_path path;
        path.move_to(p1);
        inv ? path.rarc_to(p2, r) : path.arc_to(p2, r);
        draw_path(path);
    }

protected:
    painter_context     _context;
    context_stack       _ctxst;
    int                 _width = 0;
    int                 _height = 0;
    uint                _hints = 0;
    text_image_cache    _text_image_cache;

public:
    painter() {}
    void setup_dimensions(int w, int h) { _width = w, _height = h; }
    bool query_antialias() const { return query_hints(hint_antialias); }
    void get_transform_recursively(mat3& m) const;

protected:
    void destroy_text_image_cache();
};

__ariel_end__

#endif
