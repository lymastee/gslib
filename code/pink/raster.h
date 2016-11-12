/*
 * Copyright (c) 2016 lymastee, All rights reserved.
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

#ifndef raster_08d5e09d_ab61_4fc0_a471_389a3bd240f4_h
#define raster_08d5e09d_ab61_4fc0_a471_389a3bd240f4_h

#include <pink/config.h>
#include <pink/image.h>
#include <pink/painter.h>
#include <gslib/std.h>

__pink_begin__

class painter_path;
typedef list<painter_path> painter_paths;

class painter_path
{
public:
    enum tag
    {
        pt_moveto,
        pt_lineto,
        pt_quadto,
        pt_cubicto,
    };

    class __gs_novtable node abstract
    {
    public:
        virtual ~node() {}
        virtual tag get_tag() const = 0;
        virtual const vec2& get_point() const = 0;
        virtual void set_point(const vec2&) = 0;
        virtual void interpolate(painter_linestrip& c, const node* last) const = 0;

    public:
        template<class _c>
        _c* as_node() { return static_cast<_c*>(this); }
        template<class _c>
        const _c* as_const_node() const { return static_cast<const _c*>(this); }
    };

    template<tag _tag>
    class node_tpl:
        public node
    {
    public:
        node_tpl() {}
        node_tpl(float x, float y): _pt(x, y) {}
        node_tpl(const vec2& pt): _pt(pt) {}

    public:
        tag get_tag() const override { return _tag; }
        const vec2& get_point() const override { return _pt; }
        void set_point(const vec2& pt) override { _pt = pt; }
        void interpolate(painter_linestrip& c, const node* last) const override { c.add_point(_pt); }

    protected:
        vec2        _pt;
    };

    typedef node_tpl<pt_moveto> move_to_node;
    typedef node_tpl<pt_lineto> line_to_node;

    class quad_to_node:
        public node_tpl<pt_quadto>
    {
    public:
        quad_to_node() {}
        quad_to_node(const vec2& p1, const vec2& p2): node_tpl(p2) { _c = p1; }
        void set_control(const vec2& c) { _c = c; }
        const vec2& get_control() const { return _c; }
        void interpolate(painter_linestrip& c, const node* last) const override;

    protected:
        vec2        _c;
    };

    class cubic_to_node:
        public node_tpl<pt_cubicto>
    {
    public:
        cubic_to_node() {}
        cubic_to_node(const vec2& p1, const vec2& p2, const vec2 p3): node_tpl(p3) { _c[0] = p1, _c[1] = p2; }
        void set_control1(const vec2& c) { _c[0] = c; }
        void set_control2(const vec2& c) { _c[1] = c; }
        const vec2& get_control1() const { return _c[0]; }
        const vec2& get_control2() const { return _c[1]; }
        void interpolate(painter_linestrip& c, const node* last) const override;

    protected:
        vec2        _c[2];
    };

    typedef vector<node*> node_list;
    typedef node_list::iterator iterator;
    typedef node_list::const_iterator const_iterator;
    typedef vector<int> indices;

    friend class raster;

protected:
    node_list       _nodelist;

public:
    painter_path() {}
    painter_path(const painter_path& pa) { attach(const_cast<painter_path&>(pa)); }
    ~painter_path() { destroy(); }
    bool empty() const { return _nodelist.empty(); }
    int size() const { return (int)_nodelist.size(); }
    void resize(int len);
    void destroy();
    void duplicate(const painter_path& pa);
    void attach(painter_path& pa);
    iterator begin() { return _nodelist.begin(); }
    iterator end() { return _nodelist.end(); }
    const_iterator begin() const { return _nodelist.begin(); }
    const_iterator end() const { return _nodelist.end(); }
    node* get_node(int i) { return _nodelist.at(i); }
    const node* get_node(int i) const { return _nodelist.at(i); }
    void close_path();
    void close_sub_path();
    void move_to(float x, float y) { move_to(vec2(x, y)); }
    void line_to(float x, float y) { line_to(vec2(x, y)); }
    void arc_to(float x, float y, float r) { arc_to(vec2(x, y), r); }
    void rarc_to(float x, float y, float r) { rarc_to(vec2(x, y), r); }
    void quad_to(float x1, float y1, float x2, float y2) { quad_to(vec2(x1, y1), vec2(x2, y2)); }
    void cubic_to(float x1, float y1, float x2, float y2, float x3, float y3) { cubic_to(vec2(x1, y1), vec2(x2, y2), vec2(x3, y3)); }
    void move_to(const vec2& pt) { _nodelist.push_back(gs_new(move_to_node, pt)); }
    void line_to(const vec2& pt) { _nodelist.push_back(gs_new(line_to_node, pt)); }
    void quad_to(const vec2& p1, const vec2& p2) { _nodelist.push_back(gs_new(quad_to_node, p1, p2)); }
    void cubic_to(const vec2& p1, const vec2& p2, const vec2& p3) { _nodelist.push_back(gs_new(cubic_to_node, p1, p2, p3)); }
    void arc_to(const vec2& p1, const vec2& p2, float r);
    void arc_to(const vec2& pt, float r);
    void rarc_to(const vec2& pt, float r);
    void transform(const mat3& m);
    void get_linestrips(linestrips& c) const;
    int get_control_contour(painter_linestrip& ls, int start) const;
    int get_sub_path(painter_path& sp, int start) const;
    bool is_clock_wise() const;
    bool is_convex() const;
    void simplify(painter_path& path) const;
    void tracing() const;
    void tracing_segments() const;
};

typedef painter_path::node painter_node;

struct path_info
{
    const painter_node*     node[2];

    path_info() { node[0] = node[1] = 0; }
    path_info(const painter_node* n1, const painter_node* n2) { node[0] = n1, node[1] = n2; }
    int get_order() const;
    int get_point_count() const;
    bool get_points(vec2 pt[], int cnt) const;
    void tracing() const;
};

enum curve_type
{
    ct_quad,
    ct_cubic,
};

struct __gs_novtable curve_spliter abstract
{
    vec2            fixedpt;
    float           ratio;
    curve_spliter*  child[2];
    curve_spliter*  parent;

public:
    curve_spliter();
    virtual ~curve_spliter();
    virtual curve_type get_type() const = 0;
    virtual int get_point_count() const = 0;
    virtual bool get_points(vec2 p[], int count) const = 0;
    virtual void split(float t) = 0;
    virtual void interpolate(vec2& p, float t) const = 0;
    virtual float reparameterize(const vec2& p) const = 0;
    virtual void tracing() const = 0;
    bool is_leaf() const;
};

struct curve_spliter_quad:
    public curve_spliter
{
    vec2            cp[3];
    vec3            para[2];

public:
    curve_spliter_quad(const vec2 p[3]);
    curve_type get_type() const override { return ct_quad; }
    int get_point_count() const override { return 3; }
    bool get_points(vec2 p[], int count) const override;
    void split(float t) override;
    void interpolate(vec2& p, float t) const override;
    float reparameterize(const vec2& p) const override;
    void tracing() const override;
};

struct curve_spliter_cubic:
    public curve_spliter
{
    vec2            cp[4];
    vec4            para[2];

public:
    curve_spliter_cubic(const vec2 p[4]);
    curve_type get_type() const override { return ct_cubic; }
    int get_point_count() const override { return 4; }
    bool get_points(vec2 p[], int count) const override;
    void split(float t) override;
    void interpolate(vec2& p, float t) const override;
    float reparameterize(const vec2& p) const override;
    void tracing() const override;
};

struct curve_helper
{
    static curve_spliter* create_spliter(const path_info* pnf);
    static curve_spliter* create_next_spliter(vec2& p, curve_spliter* cs, float t);
    static curve_spliter* query_spliter(curve_spliter* cs, float t);
    static curve_spliter* query_spliter(curve_spliter* cs, const vec2& p);
    static curve_spliter* query_spliter(curve_spliter* cs, const vec2& p1, const vec2& p2);
};

struct painter_helper
{
    enum
    {
        merge_straight_line     = 0x01,
        fix_loop                = 0x02,
        fix_inflection          = 0x04,
        reduce_straight_curve   = 0x08,
        reduce_short_line       = 0x10,
    };

    static void transform(painter_path& output, const painter_path& path, uint mask);
    static void close_sub_paths(painter_path& output, const painter_path& path);
};

class __gs_novtable raster abstract:
    public painterex
{
public:
    typedef painter_context context;
    typedef stack<context> context_stack;

public:
    virtual ~raster() {}
    virtual painter_version get_version() const override { return painter_ver2; }
    virtual void draw_line(const vec2& p1, const vec2& p2) override
    {
        painter_path path;
        path.move_to(p1);
        path.line_to(p2);
        draw_path(path);
    }
    virtual void draw_quad(const vec2& p1, const vec2& p2, const vec2& p3) override
    {
        painter_path path;
        path.move_to(p1);
        path.quad_to(p2, p3);
        draw_path(path);
    }
    virtual void draw_cubic(const vec2& p1, const vec2& p2, const vec2& p3, const vec2& p4) override
    {
        painter_path path;
        path.move_to(p1);
        path.cubic_to(p2, p3, p4);
        draw_path(path);
    }
    virtual void draw_arc(const vec2& p1, const vec2& p2, float r, bool inv = false) override
    {
        painter_path path;
        path.move_to(p1);
        inv ? path.rarc_to(p2, r) : path.arc_to(p2, r);
        draw_path(path);
    }

public:
    raster() { _width = _height = 0; }
    void setup_dimensions(int w, int h)
    {
        _width = w;
        _height = h;
    }

protected:
    painter_context     _context;
    context_stack       _ctxst;
    int                 _width;
    int                 _height;

public:
    virtual void save() override;
    virtual void restore() override;
    virtual void set_brush(const painter_brush& b) override { _context.set_brush(b); }
    virtual void set_pen(const painter_pen& p) override { _context.set_pen(p); }
    virtual void set_clip(const rectf& rc) override { _context.set_clip(rc); }
    virtual void set_no_clip() override { _context._clip = 0; }
    virtual context& get_context() override { return _context; }

public:
    // todo
    virtual int get_width() const override { return _width; }
    virtual int get_height() const override { return _height; }
    virtual void set_dirty(dirty_list* dirty) override {}
    virtual dirty_list* get_dirty() const override { return 0; }
    virtual image* select(image* img) override { return 0; }
    virtual image* get_image() const override { return 0; }
    virtual bool lock(const rect& rc) override { return false; }
    virtual const rect& unlock() override
    {
        static rect rc;
        return rc;
    }
    virtual void draw(const image* img, int x, int y) override {}
    virtual void draw(const image* img, int x, int y, int cx, int cy, int sx, int sy) override {}
    virtual void draw_text(const gchar* str, int x, int y, const pixel& p) override {}
    virtual void draw_line(const point& start, const point& end, const pixel& p) override {}
    virtual void draw_rect(const rect& rc, const pixel& p) override {}
};

__pink_end__

#endif
