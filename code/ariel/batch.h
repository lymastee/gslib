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

#ifndef batch_8317ae94_87f4_4cc2_8dab_1eef2919a461_h
#define batch_8317ae94_87f4_4cc2_8dab_1eef2919a461_h

#include <ariel/config.h>
#include <gslib/rtree.h>
#include <ariel/loopblinn.h>

__ariel_begin__

struct bat_triangle;
struct bat_line;
struct bat_batch;

typedef vector<bat_triangle*> bat_triangles;
typedef vector<bat_line*> bat_lines;
typedef rtree_entity<bat_triangle*> bat_rtree_entity;
typedef rtree_node<bat_rtree_entity> bat_rtree_node;
typedef _tree_allocator<bat_rtree_node> bat_rtree_alloc;
typedef tree<bat_rtree_entity, bat_rtree_node, bat_rtree_alloc> bat_tree;
typedef rtree<bat_rtree_entity, quadratic_split_alg<16, 6, bat_tree>, bat_rtree_node, bat_rtree_alloc> bat_rtree;
typedef vector<bat_batch*> bat_batches;

enum bat_type
{
    bf_start,
    bf_cr = bf_start,       /* pt, cr */
    bf_klm_cr,              /* pt, klm, cr */
    bf_tex,                 /* pt, tex */
    bf_klm_tex,             /* pt, klm, tex */
    bf_end = bf_klm_tex,

    bs_start,
    bs_coef_cr = bs_start,  /* pt, coef, cr */
    bs_coef_tex,            /* pt, coef, tex */
    bs_end = bs_coef_tex,
};

struct bat_triangle
{
    lb_joint*           _joints[3];
    vec2                _reduced[3];        /* get a reduced triangle so that no overlapping caused by error would be detected. */
    float               _zorder;
    bool                _is_reduced;

public:
    bat_triangle();
    bat_triangle(lb_joint* i, lb_joint* j, lb_joint* k);
    ~bat_triangle() {}
    void make_rect(rectf& rc);
    void set_joint(int i, lb_joint* p) { _joints[i] = p; }
    lb_joint* get_joint(int i) const { return _joints[i]; }
    const vec2& get_point(int i) const { return _joints[i]->get_point(); }
    void* get_lb_binding(int i) const { return _joints[i]->get_binding(); }
    bat_type decide(uint brush_tag) const;
    bool has_klm_coords() const;
    const vec2& get_reduced_point(int i) const { return _reduced[i]; }
    bool is_overlapped(const bat_triangle& other) const;
    void ensure_make_reduced();
    void set_zorder(float z) { _zorder = z; }
    float get_zorder() const { return _zorder; }
    void tracing() const;
    void trace_reduced_points() const;
};

struct bat_line
{
    vec2                _points[2];
    vec2                _contourpt[4];      /* calculated contour pts */
    lb_joint*           _srcjs[2];
    vec3                _coef;
    float               _width;
    float               _zorder;
    uint                _tag;               /* pen tag */
    bool                _half;              /* is half line? */
    bool                _recalc;            /* need recalculation? */

public:
    bat_line();
    void set_start_point(const vec2& p) { _points[0] = p; }
    void set_end_point(const vec2& p) { _points[1] = p; }
    void set_source_joint(int i, lb_joint* j) { _srcjs[i] = j; }
    lb_joint* get_source_joint(int i) const { return _srcjs[i]; }
    const vec2& get_start_point() const { return _points[0]; }
    const vec2& get_end_point() const { return _points[1]; }
    void set_pen_tag(uint t) { _tag = t; }
    void setup_coef();
    void set_coef(const vec3& c) { _coef = c; }
    const vec3& get_coef() const { return _coef; }
    void set_zorder(float z) { _zorder = z; }
    float get_zorder() const { return _zorder; }
    void set_line_width(float w) { _width = w; }
    float get_line_width() const { return _width; }
    void set_half_line(bool hl) { _half = hl; }
    bool is_half_line() const { return _half; }
    bat_type decide() const;
    void get_bound_rect(rectf& rc) const;
    int clip_triangle(bat_line output[2], const bat_triangle* triangle) const;
    void calc_contour_points();
    const vec2& get_contour_point(int i) const { return _contourpt[i]; }
    void set_contour_point(int i, const vec2& p) { _contourpt[i] = p; }
    bool set_need_recalc(bool b) { _recalc = b; }
    bool need_recalc() const { return _recalc; }
    void trim_contour(bat_line& line);
    void tracing() const;
};

struct bat_batch
{
    bat_type            _type;

public:
    bat_batch(bat_type t): _type(t) {}
    ~bat_batch() {}
    bat_type get_type() const { return _type; }
};

struct bat_fill_batch:
    public bat_batch
{
    bat_rtree           _rtree;

public:
    bat_fill_batch(bat_type t): bat_batch(t) {}
    bat_rtree& get_rtree() { return _rtree; }
    const bat_rtree& const_rtree() const { return _rtree; }
};

struct bat_stroke_batch:
    public bat_batch
{
    bat_lines           _lines;

public:
    bat_stroke_batch(bat_type t): bat_batch(t) {}
    bat_lines& get_lines() { return _lines; }
    const bat_lines& const_lines() const { return _lines; }
};

class batch_processor
{
public:
    typedef bat_batches::iterator bat_iter;
    typedef bat_batches::const_iterator bat_const_iter;
    friend class rose;

public:
    batch_processor();
    ~batch_processor();
    void add_polygon(lb_polygon* poly, float z, uint brush_tag);
    bat_line* add_line(lb_joint* i, lb_joint* j, float w, float z, uint pen_tag);
    bat_line* add_aa_border(lb_joint* i, lb_joint* j, float z, uint pen_tag);
    void finish_batching();
    bat_batches& get_batches() { return _batches; }
    void clear_batches();

protected:
    bat_triangles       _triangles;
    bat_lines           _lines;
    bat_batches         _batches;

protected:
    template<class _batch>
    _batch* create_batch(bat_type t);
    bat_triangle* create_triangle(lb_joint* i, lb_joint* j, lb_joint* k, float z);
    bat_line* create_line(lb_joint* i, lb_joint* j, float w, float z, uint t, bool half);
    bat_line* create_half_line(lb_joint* i, lb_joint* j, const vec2& p1, const vec2& p2, float w, float z, uint t);
    void add_triangle(lb_joint* i, lb_joint* j, lb_joint* k, bool b[3], float z, uint brush_tag);
    void collect_aa_borders(bat_triangle* triangle, bool b[3], uint pen_tag);
    void proceed_line_batch();
};

__ariel_end__

#endif
