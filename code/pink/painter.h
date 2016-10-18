/* 
 * Copyright (c) 2016 lymastee, All rights reserved.
 * Contact: lymastee@hotmail.com
 *
 * This file is part of the GSLIB project.
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

#include <gslib/sysop.h>
#include <pink/image.h>

__pink_begin__

enum painter_version
{
    painter_ver1,       /* software painter */
    painter_ver2,       /* rose painter */
    painter_ver3,       /* not used. */
};

class __gs_novtable painter abstract
{
public:
    virtual painter_version get_version() const = 0;
    virtual int get_width() const = 0;
    virtual int get_height() const = 0;
    virtual void set_dirty(dirty_list* dirty) = 0;
    virtual dirty_list* get_dirty() const = 0;
    virtual image* select(image* img) = 0;
    virtual image* get_image() const = 0;
    virtual bool lock(const rect& rc) = 0;
    virtual const rect& unlock() = 0;
    virtual void on_draw_begin() = 0;
    virtual void on_draw_end() = 0;
    virtual void draw(const image* img, int x, int y) = 0;
    virtual void draw(const image* img, int x, int y, int cx, int cy, int sx, int sy) = 0;
    virtual void draw_text(const gchar* str, int x, int y, const pixel& p) = 0;
    virtual void draw_line(const point& start, const point& end, const pixel& p) = 0;
    virtual void draw_rect(const rect& rc, const pixel& p) = 0;
};

class painter_linestrip
{
public:
    typedef vector<vec2> points;
    typedef points::iterator ptiter;
    typedef points::const_iterator ptciter;

protected:
    bool            _closed;
    points          _pts;

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

protected:
    enum
    {
        st_clockwise_mask       = 1,
        st_convex_mask          = 2,
    };
    mutable uint    _tst_table;
    mutable bool    _is_clock_wise;
    mutable bool    _is_convex;

protected:
    bool is_clockwise_init() const { return (_tst_table & st_clockwise_mask) != 0; }
    void set_clockwise_init() const { _tst_table |= st_clockwise_mask; }
    bool is_convex_init() const { return (_tst_table & st_convex_mask) != 0; }
    void set_convex_init() const { _tst_table |= st_convex_mask; }
};

typedef list<painter_linestrip> linestrips;
typedef vector<painter_linestrip*> linestripvec;

struct gradient {};

struct painter_brush
{
    enum
    {
        null,
        solid,
    };
    uint            _tag;
    uint            _extra;
    color           _color;

    typedef void (*fn_fill)(image& img, linestrips& c, const pixel& cr, uint ext);
    typedef void (*fn_fill_alpha)(image& img, linestrips& c, const pixel& cr, float alpha, uint ext);
    static bool preprocess_linestrips(linestrips& c);
    static void solid_fill(image& img, linestrips& c, const pixel& cr, uint ext);

public:
    painter_brush() { _tag = null; }
    void set_tag(uint tag) { _tag = tag; }
    uint get_tag() const { return _tag; }
    void set_color(const color& cr) { _color = cr; }
    const color& get_color() const { return _color; }
    void set_extra(uint ext) { _extra = ext; }
    uint get_extra() const { return _extra; }
    void fill(image& img, linestrips& c, float alpha = 1.f) const;
};

struct painter_pen
{
    enum
    {
        null,
        solid,
    };
    uint            _tag;
    uint            _extra;
    color           _color;

    typedef void (*fn_stroke)(image& img, const pointf& p1, const pointf& p2, const pixel& cr, uint ext);
    typedef void (*fn_stroke_alpha)(image& img, const pointf& p1, const pointf& p2, const pixel& cr, float alpha, uint ext);
    static void solid_stroke(image& img, const pointf& p1, const pointf& p2, const pixel& cr, uint ext);
    static void solid_stroke_alpha(image& img, const pointf& p1, const pointf& p2, const pixel& cr, float alpha, uint ext);
    // more to come...

public:
    painter_pen() { _tag = null; }
    painter_pen(uint tag) { _tag = tag; }
    painter_pen(uint tag, const color& cr) { _tag = tag; _color = cr; }
    void set_tag(uint tag) { _tag = tag; }
    uint get_tag() const { return _tag; }
    void set_color(const color& cr) { _color = cr; }
    const color& get_color() const { return _color; }
    void set_extra(uint ext) { _extra = ext; }
    uint get_extra() const { return _extra; }
};

struct painter_context
{
    painter_brush       _brush;
    painter_pen         _pen;
    rectf*              _clip;
    rectf               _clipdata;

    painter_context() { _clip = 0; }
    void set_brush(const painter_brush& brush) { _brush = brush; }
    void set_pen(const painter_pen& pen) { _pen = pen; }
    void set_clip(const rectf& rc)
    {
        _clipdata = rc;
        _clip = &_clipdata;
    }
    const painter_brush& get_brush() const { return _brush; }
    const painter_pen& get_pen() const { return _pen; }
};

class painter_path;

class __gs_novtable painterex abstract:
    public painter
{
public:
    virtual void resize(int w, int h) = 0;
    virtual void set_clip(const rectf& rc) = 0;
    virtual void set_no_clip() = 0;
    virtual void set_brush(const painter_brush& b) = 0;
    virtual void set_pen(const painter_pen& p) = 0;
    virtual void draw_path(const painter_path& path) = 0;
    virtual void draw_line(const vec2& p1, const vec2& p2) = 0;
    virtual void draw_quad(const vec2& p1, const vec2& p2, const vec2& p3) = 0;
    virtual void draw_cubic(const vec2& p1, const vec2& p2, const vec2& p3, const vec2& p4) = 0;
    virtual void draw_arc(const vec2& p1, const vec2& p2, float r, bool inv = false) = 0;
    virtual void save() = 0;
    virtual void restore() = 0;
    virtual painter_context& get_context() = 0;
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
    painter_obj*    _parent;
    obj_list        _children;
    painter_context _context;

public:
    virtual ~painter_obj();
    virtual void draw() = 0;
};

class software_painter:
    public painter
{
protected:
    rect            _lock;
    bool            _locked;
    image*          _image;
    dirty_list*     _dirty;

public:
    software_painter();
    virtual ~software_painter();
    virtual painter_version get_version() const override { return painter_ver1; }
    virtual int get_width() const override { return _image->get_width(); }
    virtual int get_height() const override { return _image->get_height(); }
    virtual void set_dirty(dirty_list* dirty) override { _dirty = dirty; }
    virtual dirty_list* get_dirty() const override { return _dirty; }
    virtual image* select(image* ptr) override;
    virtual image* get_image() const override { return _image; }
    virtual bool lock(const rect& rc) override;
    virtual const rect& unlock() override;
    virtual void on_draw_begin() override {}
    virtual void on_draw_end() override {}
    virtual void draw(const image* img, int x, int y) override { draw(img, x, y, img->get_width(), img->get_height(), 0, 0); }
    virtual void draw(const image* img, int x, int y, int cx, int cy, int sx, int sy) override;
    virtual void draw_text(const gchar* str, int x, int y, const pixel& p) override;
    virtual void draw_line(const point& start, const point& end, const pixel& p) override;
    virtual void draw_rect(const rect& rc, const pixel& p) override;
};

struct select_software_painter
{
    static painter& get_painter()
    {
        static software_painter inst;
        return inst;
    }
};

__pink_end__

#endif
