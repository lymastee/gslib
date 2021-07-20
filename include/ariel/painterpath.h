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

#ifndef painterpath_d2d84280_c369_48f2_99f8_28652ad13baa_h
#define painterpath_d2d84280_c369_48f2_99f8_28652ad13baa_h

#include <ariel/config.h>
#include <gslib/std.h>

__ariel_begin__

class painter_path;
typedef list<painter_path> painter_paths;

class ariel_export painter_linestrip
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
    void get_bound_rect(rectf& rc) const;
    vec2& get_point(int i) { return _pts.at(i); }
    const vec2& get_point(int i) const { return _pts.at(i); }
    vec2& get_last_point() { return _pts.back(); }
    const vec2& get_last_point() const { return _pts.back(); }
    void add_point(const vec2& pt) { _pts.push_back(pt); }
    void clear() { _pts.clear(); }
    void swap(painter_linestrip& another);
    void finish();
    void transform(const mat3& m);
    vec2* expand(int size);
    void expand_to(int size) { _pts.resize(size); }
    void reverse();
    int point_inside(const vec2& pt) const;
    bool is_closed() const { return _closed; }
    void set_closed(bool c) { _closed = c; }
    bool is_clockwise() const;
    bool is_convex() const;
    bool is_convex(int i) const;
    void tracing() const;
    void tracing_segments() const;

protected:
    enum
    {
        st_clockwise_mask = 1,
        st_convex_mask = 2,
    };
    mutable uint        _tst_table;
    mutable bool        _is_clock_wise;
    mutable bool        _is_convex;

protected:
    bool is_clockwise_inited() const { return (_tst_table & st_clockwise_mask) != 0; }
    void set_clockwise_inited() const { _tst_table |= st_clockwise_mask; }
    bool is_convex_inited() const { return (_tst_table & st_convex_mask) != 0; }
    void set_convex_inited() const { _tst_table |= st_convex_mask; }
};

typedef list<painter_linestrip> linestrips;
typedef vector<painter_linestrip*> linestripvec;
typedef linestrips painter_linestrips;

/* create a random access view for linestrips */
ariel_export extern void append_linestrips_rav(linestripvec& rav, linestrips& src);
ariel_export extern void create_linestrips_rav(linestripvec& rav, linestrips& src);

class ariel_export painter_path
{
public:
    enum tag
    {
        pt_moveto,
        pt_lineto,
        pt_quadto,
        pt_cubicto,
    };

    class __gs_novtable ariel_export node abstract
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
        vec2            _pt;
    };

    typedef node_tpl<pt_moveto> move_to_node;
    typedef node_tpl<pt_lineto> line_to_node;

    class ariel_export quad_to_node:
        public node_tpl<pt_quadto>
    {
    public:
        quad_to_node() {}
        quad_to_node(const vec2& p1, const vec2& p2): node_tpl(p2) { _c = p1; }
        void set_control(const vec2& c) { _c = c; }
        const vec2& get_control() const { return _c; }
        void interpolate(painter_linestrip& c, const node* last) const override;

    protected:
        vec2            _c;
    };

    class ariel_export cubic_to_node:
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
        vec2            _c[2];
    };

    typedef vector<node*> node_list;
    typedef node_list::iterator iterator;
    typedef node_list::const_iterator const_iterator;
    typedef vector<int> indices;

protected:
    node_list           _nodelist;

public:
    painter_path() {}
    painter_path(const painter_path& path) { duplicate(path); }
    ~painter_path() { destroy(); }
    bool empty() const { return _nodelist.empty(); }
    int size() const { return (int)_nodelist.size(); }
    void resize(int len);
    void destroy();
    void duplicate(const painter_path& path);
    void add_path(const painter_path& path);
    void add_rect(const rectf& rc);
    void swap(painter_path& path);
    iterator begin() { return _nodelist.begin(); }
    iterator end() { return _nodelist.end(); }
    const_iterator begin() const { return _nodelist.begin(); }
    const_iterator end() const { return _nodelist.end(); }
    node* get_node(int i) { return _nodelist.at(i); }
    const node* get_node(int i) const { return _nodelist.at(i); }
    void close_path();
    void close_sub_path();
    void get_boundary_box(rectf& rc) const;
    void move_to(float x, float y) { move_to(vec2(x, y)); }
    void line_to(float x, float y) { line_to(vec2(x, y)); }
    void arc_to(float x, float y, float r) { arc_to(vec2(x, y), r); }
    void rarc_to(float x, float y, float r) { rarc_to(vec2(x, y), r); }
    void quad_to(float x1, float y1, float x2, float y2) { quad_to(vec2(x1, y1), vec2(x2, y2)); }
    void cubic_to(float x1, float y1, float x2, float y2, float x3, float y3) { cubic_to(vec2(x1, y1), vec2(x2, y2), vec2(x3, y3)); }
    void move_to(const vec2& pt) { _nodelist.push_back(new move_to_node(pt)); }
    void line_to(const vec2& pt) { _nodelist.push_back(new line_to_node(pt)); }
    void quad_to(const vec2& p1, const vec2& p2) { _nodelist.push_back(new quad_to_node(p1, p2)); }
    void cubic_to(const vec2& p1, const vec2& p2, const vec2& p3) { _nodelist.push_back(new cubic_to_node(p1, p2, p3)); }
    void arc_to(const vec2& p1, const vec2& p2, float r);
    void arc_to(const vec2& pt, float r);
    void rarc_to(const vec2& pt, float r);
    void transform(const mat3& m);
    void get_linestrips(linestrips& c) const;
    int get_control_contour(painter_linestrip& ls, int start) const;
    int get_sub_path(painter_path& sp, int start) const;
    void to_sub_paths(painter_paths& paths) const;
    bool is_clockwise() const;
    bool is_convex() const;
    void simplify(painter_path& path) const;
    void reverse();
    void tracing() const;
    void tracing_segments() const;
};

typedef painter_path::node painter_node;

struct ariel_export path_info
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

struct __gs_novtable ariel_export curve_splitter abstract
{
    vec2                fixedpt;
    float               ratio;
    curve_splitter*     child[2];
    curve_splitter*     parent;

public:
    curve_splitter();
    virtual ~curve_splitter();
    virtual curve_type get_type() const = 0;
    virtual int get_point_count() const = 0;
    virtual bool get_points(vec2 p[], int count) const = 0;
    virtual void split(float t) = 0;
    virtual void interpolate(vec2& p, float t) const = 0;
    virtual float reparameterize(const vec2& p) const = 0;
    virtual void tracing() const = 0;
    bool is_leaf() const;
};

struct ariel_export curve_splitter_quad:
    public curve_splitter
{
    vec2                cp[3];
    vec3                para[2];

public:
    curve_splitter_quad(const vec2 p[3]);
    curve_type get_type() const override { return ct_quad; }
    int get_point_count() const override { return 3; }
    bool get_points(vec2 p[], int count) const override;
    void split(float t) override;
    void interpolate(vec2& p, float t) const override;
    float reparameterize(const vec2& p) const override;
    void tracing() const override;
};

struct ariel_export curve_splitter_cubic:
    public curve_splitter
{
    vec2                cp[4];
    vec4                para[2];

public:
    curve_splitter_cubic(const vec2 p[4]);
    curve_type get_type() const override { return ct_cubic; }
    int get_point_count() const override { return 4; }
    bool get_points(vec2 p[], int count) const override;
    void split(float t) override;
    void interpolate(vec2& p, float t) const override;
    float reparameterize(const vec2& p) const override;
    void tracing() const override;
};

struct ariel_export curve_helper
{
    static curve_splitter* create_splitter(const path_info* pnf);
    static curve_splitter* create_next_splitter(vec2& p, curve_splitter* cs, float t);
    static curve_splitter* query_splitter(curve_splitter* cs, float t);
    static curve_splitter* query_splitter(curve_splitter* cs, const vec2& p);
    static curve_splitter* query_splitter(curve_splitter* cs, const vec2& p1, const vec2& p2);
};

struct ariel_export painter_helper
{
    enum
    {
        merge_straight_line = 0x01,
        fix_loop = 0x02,
        fix_inflection = 0x04,
        reduce_straight_curve = 0x08,
        reduce_short_line = 0x10,
    };

    static void transform(painter_path& output, const painter_path& path, uint mask);
    static void close_sub_paths(painter_path& output, const painter_path& path);
};

__ariel_end__

#endif
