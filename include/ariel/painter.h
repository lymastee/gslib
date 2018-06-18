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

#ifndef painter_0e5e866a_20d2_4fb1_93c0_1785588121f2_h
#define painter_0e5e866a_20d2_4fb1_93c0_1785588121f2_h

#include <memory>
#include <ariel/sysop.h>
#include <ariel/image.h>

__ariel_begin__

class painter_linestrip
{
public:
    typedef vector<vec2> points;
    typedef points::iterator ptiter;
    typedef points::const_iterator ptciter;

protected:
    bool                _closed;
    points              _pts;

public:
    painter_linestrip();
    ~painter_linestrip() {}
    int get_size() const { return (int)_pts.size(); }
    vec2& get_point(int i) { return _pts.at(i); }
    const vec2& get_point(int i) const { return _pts.at(i); }
    void add_point(const vec2& pt) { _pts.push_back(pt); }
    void clear() { _pts.clear(); }
    void swap(painter_linestrip& another);
    void finish();
    vec2* expand(int size);
    void expand_to(int size) { _pts.resize(size); }
    bool is_closed() const { return _closed; }
    void set_closed(bool c) { _closed = c; }
    bool is_clock_wise() const;
    bool is_convex() const;
    bool is_convex(int i) const;
    void tracing() const;

protected:
    enum
    {
        st_clockwise_mask       = 1,
        st_convex_mask          = 2,
    };
    mutable uint        _tst_table;
    mutable bool        _is_clock_wise;
    mutable bool        _is_convex;

protected:
    bool is_clockwise_init() const { return (_tst_table & st_clockwise_mask) != 0; }
    void set_clockwise_init() const { _tst_table |= st_clockwise_mask; }
    bool is_convex_init() const { return (_tst_table & st_convex_mask) != 0; }
    void set_convex_init() const { _tst_table |= st_convex_mask; }
};

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

public:
    painter_picture_data() { _image = nullptr; }
    painter_picture_data(texture2d* p) { _image = p; }
    void set_image(texture2d* p) { _image = p; }
    texture2d* get_image() const { return _image; }
};

typedef list<painter_linestrip> linestrips;
typedef vector<painter_linestrip*> linestripvec;
typedef std::shared_ptr<painter_data> painter_extra_data;

struct gradient {};

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
        hint_anti_alias     = 0x1,
    };
    typedef vector<texture2d*> text_image_cache;

public:
    virtual ~painter();
    virtual int get_width() const = 0;
    virtual int get_height() const = 0;
    virtual void set_dirty(dirty_list* dirty) = 0;
    virtual dirty_list* get_dirty() const = 0;
    virtual void resize(int w, int h) = 0;
    virtual void set_hints(uint hints, bool enable) = 0;
    virtual void set_brush(const painter_brush& b) = 0;
    virtual void set_pen(const painter_pen& p) = 0;
    virtual void set_tranform(const mat3& m) = 0;
    virtual void draw_path(const painter_path& path) = 0;
    virtual void draw_line(const vec2& p1, const vec2& p2) = 0;
    virtual void draw_rect(const rectf& rc) = 0;
    virtual void draw_quad(const vec2& p1, const vec2& p2, const vec2& p3) = 0;
    virtual void draw_cubic(const vec2& p1, const vec2& p2, const vec2& p3, const vec2& p4) = 0;
    virtual void draw_arc(const vec2& p1, const vec2& p2, float r, bool inv = false) = 0;
    virtual void save() = 0;
    virtual void restore() = 0;
    virtual bool query_hints(uint hints) const = 0;
    virtual painter_context& get_context() = 0;

public:
    virtual void on_draw_begin();
    virtual void on_draw_end();
    virtual void draw_image(texture2d* img, float x, float y);
    virtual void draw_line(const vec2& p1, const vec2& p2, const color& cr);
    virtual void draw_rect(const rectf& rc, const color& cr);
    virtual void draw_text(const gchar* str, int x, int y, const color& cr, int length = -1);

protected:
    text_image_cache    _text_image_cache;

protected:
    void destroy_text_image_cache();
};

class painter_obj;
typedef list<painter_obj*> painter_obj_list;
typedef painter_obj_list::iterator painter_obj_iter;
typedef painter_obj_list::const_iterator painter_obj_const_iter;

class __gs_novtable painter_obj abstract
{
public:
    typedef painter_obj_list obj_list;
    typedef painter_obj_iter iterator;
    typedef painter_obj_const_iter const_iterator;

public:
    painter_obj(const painter_context& ctx);
    void set_parent(painter_obj* p) { _parent = p; }
    painter_obj* get_parent() const { return _parent; }
    void set_context(const painter_context& c) { _context = c; }
    const painter_context& get_context() const { return _context; }
    painter_obj_iter add_child(painter_obj* p);
    painter_obj_iter find_child_iterator(painter_obj* p);
    painter_obj_iter add_child_before(painter_obj_iter pos, painter_obj* p);
    painter_obj_iter add_child_after(painter_obj_iter pos, painter_obj* p);
    void detach_child(painter_obj_iter pos);
    void detach_child(painter_obj* p);
    void remove_child(painter_obj_iter pos);
    void remove_child(painter_obj* p);
    painter_obj_iter get_self_iterator();
    void destroy_children();

protected:
    painter_obj*        _parent;
    obj_list            _children;
    painter_context     _context;

public:
    virtual ~painter_obj();
    virtual void draw() = 0;
};

__ariel_end__

#endif
