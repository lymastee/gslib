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

#ifndef loopblinn_875a5488_30b0_4449_b97a_e55c8d8a0db1_h
#define loopblinn_875a5488_30b0_4449_b97a_e55c8d8a0db1_h

#include <gslib/std.h>
#include <gslib/rtree.h>
#include <pink/raster.h>
#include <ariel/delaunay.h>

__ariel_begin__

using pink::raster;
using pink::image;
using pink::painter_path;
using pink::painter_node;
using pink::color;
using pink::painter_brush;
using pink::painter_pen;

class lb_joint;
class lb_end_joint;
class lb_control_joint;
class lb_line;
class lb_polygon;
class lb_bisp_line;
class lb_bisp;

typedef vector<lb_joint*> lb_joint_list;
typedef vector<lb_line*> lb_line_list;
typedef vector<lb_polygon*> lb_polygon_list;
typedef vector<lb_bisp_line*> lb_bisp_lines;
typedef stack<lb_polygon*> lb_polygon_stack;

enum lb_joint_type
{
    lbt_end_joint,
    lbt_control_joint,
};

/*
 * The reason why the original point and the ncoord(normalized coordinate) point should be
 * separated was that we need the original point for the convenience of the error handling
 * works, and we need the ncoord point to do the loopblinn calculations so that the float
 * point won't overflow.
 */
class __gs_novtable lb_joint abstract
{
protected:
    lb_line*            _prev;
    lb_line*            _next;
    vec2                _point;
    vec2                _ncoord;
    void*               _binding;

public:
    lb_joint()
    {
        _prev = _next = 0;
        _binding = 0;
    }
    virtual ~lb_joint() {}
    virtual lb_joint_type get_type() const = 0;
    virtual const vec2& get_point() const { return _point; }
    virtual const vec2& get_ncoord_point() const { return _ncoord; }

public:
    void set_point(const vec2& p) { _point = p; }
    void set_ncoord_point(const vec2& p) { _ncoord = p; }
    void set_prev_line(lb_line* p) { _prev = p; }
    void set_next_line(lb_line* p) { _next = p; }
    lb_line* get_prev_line() const { return _prev; }
    lb_line* get_next_line() const { return _next; }
    lb_joint* get_prev_joint() const;
    lb_joint* get_next_joint() const;
    bool is_adjacent_joint(const lb_joint* p) const { return p == get_prev_joint() || p == get_next_joint(); }
    void set_binding(void* p) { _binding = p; }
    void* get_binding() const { return _binding; }
};

class lb_end_joint:
    public lb_joint
{
protected:
    vec3                _klm[2];

public:
    lb_end_joint() {}
    lb_joint_type get_type() const override { return lbt_end_joint; }
    bool prev_is_curve() const;
    bool next_is_curve() const;
    void set_klm(int i, const vec3& p) { _klm[i] = p; }
    const vec3& get_klm(int i) const { return _klm[i]; }
};

class lb_control_joint:
    public lb_joint
{
protected:
    vec3                _klm;

public:
    lb_control_joint() {}
    lb_joint_type get_type() const override { return lbt_control_joint; }
    void set_klm(const vec3& p) { _klm = p; }
    const vec3& get_klm() const { return _klm; }
};

class lb_line
{
protected:
    lb_joint*           _joint[2];
    bool                _opened;

public:
    lb_line()
    {
        _joint[0] = _joint[1] = 0;
        _opened = false;
    }
    void set_opened(bool b) { _opened = b; }
    bool is_opened() const { return _opened; }
    void set_prev_joint(lb_joint* p) { _joint[0] = p; }
    void set_next_joint(lb_joint* p) { _joint[1] = p; }
    lb_joint* get_prev_joint() const { return _joint[0]; }
    lb_joint* get_next_joint() const { return _joint[1]; }
    lb_line* get_prev_line() const;
    lb_line* get_next_line() const;
    const vec2& get_prev_point() const { return _joint[0]->get_point(); }
    const vec2& get_next_point() const { return _joint[1]->get_point(); }
    static lb_line* get_line_between(lb_joint* j1, lb_joint* j2);
};

class lb_bisp_line
{
protected:
    lb_bisp_line*       _prev;
    lb_bisp_line*       _next;
    lb_bisp_line*       _bin[2];
    vec2                _point;
    vec3                _le;

public:
    lb_bisp_line();
    void set_prev(lb_bisp_line* p) { _prev = p; }
    void set_next(lb_bisp_line* p) { _next = p; }
    void set_binary(int i, lb_bisp_line* p) { _bin[i] = p; }
    void set_prev_point(const vec2& p) { _point = p; }
    void set_next_point(const vec2& p) { _next->set_prev_point(p); }
    lb_bisp_line* get_prev() const { return _prev; }
    lb_bisp_line* get_next() const { return _next; }
    lb_bisp_line* get_binary(int i) const { return _bin[i]; }
    const vec2& get_prev_point() const { return _point; }
    const vec2& get_next_point() const { return _next->get_prev_point(); }
    bool is_boundary() const { return !_bin[0]; }
    bool is_le_available() const;
    void calc_linear_expression();
    float get_le_dot(const vec2& p) const;
    void tracing() const;
};

class lb_bisp
{
public:
    lb_bisp() { _center = 0; }
    ~lb_bisp();
    lb_bisp_line* get_center() const { return _center; }
    void setup(lb_line* start);
    bool is_inside(const vec2& p) const;
    void tracing() const;
    void trace_loop() const;

protected:
    lb_bisp_lines       _lines;
    lb_bisp_line*       _center;

protected:
    lb_bisp_line* create_line();
    lb_bisp_line* create(lb_line* start);
    void create_segment(lb_line_list& span, lb_bisp_line* seg[2]);
    void create_linear_segment(lb_line* line1, lb_bisp_line* seg[2]);
    void create_quadratic_segment(lb_line* line1, lb_line* line2, lb_bisp_line* seg[2]);
    void create_cubic_segment(lb_line* line1, lb_line* line2, lb_line* line3, lb_bisp_line* seg[2]);
    lb_bisp_line* shrink(lb_bisp_line* start);
    lb_bisp_line* make_shrink(lb_bisp_line* line1, lb_bisp_line* line2);
    lb_bisp_line* query(const vec2& p) const;
    lb_bisp_line* query(const vec2& p, lb_bisp_line* last) const;
};

enum lb_span_type
{
    lst_linear,
    lst_quad,
    lst_cubic,
};

class __gs_novtable lb_span abstract
{
protected:
    rectf               _rc;

public:
    virtual ~lb_span() {}
    virtual lb_span_type get_type() const = 0;
    virtual bool can_split() const = 0;
    virtual bool is_overlapped(const lb_span* span) const = 0;

public:
    const rectf& get_rect() const { return _rc; }
    float get_area() const { return _rc.width() * _rc.height(); }
};

typedef vector<lb_span*> lb_span_list;
typedef rtree_entity<lb_span*> lb_rtree_entity;
typedef rtree_node<lb_rtree_entity> lb_rtree_node;
typedef _tree_allocator<lb_rtree_node> lb_rtree_alloc;
typedef tree<lb_rtree_entity, lb_rtree_node, lb_rtree_alloc> lb_tree;
typedef rtree<lb_rtree_entity, quadratic_split_alg<8, 3, lb_tree>, lb_rtree_node, lb_rtree_alloc> lb_rtree;
typedef delaunay_triangulation lb_triangulator;

/*
 * The polygon should be decomposed to the form like boundary - holes,
 * this procedure could also be called hierarchy flatten.
 * After we retrieve the boundary we need to create a bisp for the boundary
 * for including test, if necessary, so that we could decide if a sub path
 * (case : cw - ccw - {cw}?) was a new path that ends the previous boundary.
 */
class lb_polygon
{
protected:
    lb_line*            _boundary;
    lb_line_list        _holes;
    lb_bisp             _bisp;
    lb_rtree            _mytree;
    lb_triangulator     _cdt;
    dt_input_joints     _dtjoints;

public:
    lb_polygon() { _boundary = 0; }
    void set_boundary(lb_line* p) { _boundary = p; }
    void add_hole(lb_line* p) { _holes.push_back(p); }
    lb_line* get_boundary() const { return _boundary; }
    const lb_line_list& get_holes() const { return _holes; }
    bool is_bisp_available() const { return _bisp.get_center() != 0; }
    void create_bisp() { _bisp.setup(_boundary); }
    void ensure_create_bisp() { if(!is_bisp_available()) create_bisp(); }
    bool is_inside(const vec2& p) const { return _bisp.is_inside(p); }
    lb_rtree& get_rtree() { return _mytree; }
    void convert_to_ncoord(const mat3& m);
    void create_dt_joints();
    void add_trim_constraints();
    void pack_constraints1();
    void pack_constraints();
    void build_cdt();
    lb_triangulator& get_cdt_result() { return _cdt; }
    void tracing() const;
    void trace_boundary() const;
    void trace_last_hole() const;
    void trace_holes() const;
    void trace_bisp() const { _bisp.tracing(); }
    void trace_rtree() const { _mytree.tracing(); }
};

/*
 * Notice that the path put into the loopblinn categorizer MUST be a simple polygon,
 * which means you should run a xor clip process before if you can't tell whether a
 * path was simple or complex.
 * Another point was that the path MUST be Winding rule.
 * You can also convert a path of OddEven rule to Winding by clipping.
 */
class loop_blinn_processor
{
public:
    loop_blinn_processor(float w, float h) { _width = w, _height = h; }
    ~loop_blinn_processor();
    void proceed(const painter_path& path);
    lb_polygon_list& get_polygons() { return _polygons; }
    lb_joint_list& get_joints() { return _joint_holdings; }
    lb_line_list& get_lines() { return _line_holdings; }
    void trace_polygons() const;
    void trace_bisps() const;
    void trace_rtree() const;

protected:
    float               _width;
    float               _height;
    lb_line_list        _line_holdings;
    lb_joint_list       _joint_holdings;
    lb_polygon_list     _polygons;
    lb_span_list        _span_holdings;

protected:
    template<class _joint>
    lb_joint* create_joint(const vec2& p);
    lb_line* create_line();
    lb_polygon* create_polygon();
    void hierarchy_flatten(const painter_path& path);
    int hierarchy_flatten(const painter_path& path, int start, lb_polygon* parent, lb_polygon_stack& st);
    int create_patch(lb_line*& line, const painter_path& path, int start);
    lb_joint* create_segment(lb_joint* prev, const painter_path& path, int i);
    void check_boundary(lb_polygon* poly);
    void check_holes(lb_polygon* poly);
    void check_span(lb_polygon* poly, lb_control_joint* joint);
    void check_rtree(lb_polygon* poly);
    void check_rtree_span(lb_polygon* poly, lb_span* span);
    void split_quadratic(lb_line* line1, lb_line* line2, lb_joint* sp[5]);
    void split_cubic(lb_line* line1, lb_line* line2, lb_line* line3, lb_joint* sp[7], float t);
    int try_split_cubic(lb_line* line1, lb_line* line2, lb_line* line3, lb_joint* sp[7], float t);
    lb_span* split_rtree_span(lb_span* span);
    void split_span_recursively(lb_polygon* poly, lb_span* span);
    void calc_klm_coords();
    void calc_klm_coords(lb_polygon* poly);
    void calc_klm_coords(lb_polygon* poly, lb_line* start);
    lb_line* calc_klm_span(lb_polygon* poly, lb_line* line);
};

extern lb_line* lb_get_span(lb_line_list& span, lb_line* start);
extern void lb_get_current_span(lb_line_list& span, lb_control_joint* joint);

__ariel_end__

#endif
