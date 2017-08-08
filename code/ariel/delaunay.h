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

#ifndef delaunay_3673850c_175d_4077_bef2_fc69e19e5cf8_h
#define delaunay_3673850c_175d_4077_bef2_fc69e19e5cf8_h

#include <gslib/std.h>
#include <gslib/math.h>
#include <ariel/config.h>

/*
 * A implementation of the Delaunay Triangulation and the Constrained Delaunay Triangulation.
 * The original implementation was:
 * https://code.google.com/archive/p/cgprojects09/
 * further more, modifications were done to achieve the constrained version, visit:
 * http://www.geom.uiuc.edu/~samuelp/del_project.html
 */

__ariel_begin__

class dt_joint
{
protected:
    vec2            _point;
    void*           _binding;

public:
    dt_joint() { _binding = 0; }
    dt_joint(const vec2& p, void* b) { _point = p, _binding = b; }
    void set_point(const vec2& p) { _point = p; }
    const vec2& get_point() const { return _point; }
    void set_binding(void* b) { _binding = b; }
    void* get_binding() const { return _binding; }
};

class dt_edge
{
protected:
    dt_joint*       _org;
    dt_edge*        _prev;
    dt_edge*        _next;
    dt_edge*        _symmetric;
    bool            _constraint;
    bool            _checked;

public:
    dt_edge();
    void set_org(dt_joint* j) { _org = j; }
    dt_joint* get_org() const { return _org; }
    void set_dest(dt_joint* j) { _symmetric->set_org(j); }
    dt_joint* get_dest() const { return _symmetric->get_org(); }
    void set_prev_edge(dt_edge* e) { _prev = e; }
    dt_edge* get_prev_edge() const { return _prev; }
    void set_next_edge(dt_edge* e) { _next = e; }
    dt_edge* get_next_edge() const { return _next; }
    void set_symmetric(dt_edge* e) { _symmetric = e; }
    dt_edge* get_symmetric() const { return _symmetric; }
    const vec2& get_org_point() const { return _org->get_point(); }
    const vec2& get_dest_point() const { return get_dest()->get_point(); }
    void* get_org_binding() const { return _org->get_binding(); }
    void* get_dest_binding() const { return get_dest()->get_binding(); }
    dt_edge* get_org_next() const { return _prev->get_symmetric(); }
    dt_edge* get_org_prev() const { return _symmetric->get_next_edge(); }
    dt_edge* get_dest_next() const { return _symmetric->get_prev_edge(); }
    dt_edge* get_dest_prev() const { return _next->get_symmetric(); }
    dt_edge* get_left_next() const { return _next; }
    dt_edge* get_left_prev() const { return _prev; }
    dt_edge* get_right_next() const { return _symmetric->get_next_edge()->get_symmetric(); }
    dt_edge* get_right_prev() const { return _symmetric->get_prev_edge()->get_symmetric(); }
    void set_constraint(bool b) { _constraint = b; }
    bool is_constraint() const { return _constraint; }
    void set_checked(bool c) { _checked = c; }
    bool is_checked() const { return _checked; }
    bool is_outside_boundary() const;
    bool is_boundary() const;
};

struct dt_edge_range
{
    dt_edge*        left;
    dt_edge*        right;

    dt_edge_range()  { left = right = 0; }
};

typedef list<dt_joint> dt_input_joints;
typedef list<dt_joint*> dt_joint_ptrs;
typedef vector<dt_joint*> dt_joints;
typedef unordered_set<dt_edge*> dt_edges;
typedef vector<dt_edge*> dt_edge_list;

class delaunay_triangulation
{
public:
    delaunay_triangulation() {}
    ~delaunay_triangulation() { clear(); }
    void initialize(dt_input_joints& inputs);
    void run();
    void clear();
    dt_edge* add_constraint(const vec2& p1, const vec2& p2);
    void trim(dt_edge_list& edges);
    void set_range_left(dt_edge* e) { _edge_range.left = e; }
    void tracing() const;
    void trace_heuristically() const;

protected:
    dt_joints       _sorted_joints;
    dt_edge_range   _edge_range;
    dt_edges        _edge_holdings;

protected:
    dt_edge_range delaunay(int begin, int end);
    dt_edge* create_edge_pair();
    dt_edge* connect_edges(dt_edge* e1, dt_edge* e2);
    void destroy_edge_pair(dt_edge* e);
    void shrink_triangulate(dt_edge* cut);
    void shrink_recursively(dt_edge_list& strips, dt_edge_list& temps);
    void collect_trim_edges(dt_edge_list& edges, dt_edge* e);

    template<class _visit>
    static void traverse_triangles_recursively(dt_edge* e, _visit visit)
    {
        assert(e);
        if(e->is_checked())
            return;
        auto do_visit = [&visit](dt_edge* e) {
            assert(e);
            auto* e1 = e->get_prev_edge();
            auto* e2 = e->get_next_edge();
            assert(e1 && e2 && !e1->is_checked() && !e2->is_checked());
            void* b1 = e->get_org_binding();
            void* b2 = e->get_dest_binding();
            void* b3 = e1->get_org_binding();
            void* b4 = e2->get_dest_binding();
            if(b3 != b4)    /* invalid triangle */
                return;
            auto& p1 = e->get_org_point();
            auto& p2 = e->get_dest_point();
            auto& p3 = e1->get_org_point();
            if(!pink::is_concave_angle(p1, p2, p3))     /* ccw! */
                return;
            bool b[3];
            b[0] = e->is_boundary();
            b[1] = e1->is_boundary();
            b[2] = e2->is_boundary();
            visit(b1, b2, b3, b);
            e->set_checked(true);
            e1->set_checked(true);
            e2->set_checked(true);
        };
        do_visit(e);
        traverse_triangles_recursively(e->get_symmetric(), visit);
        traverse_triangles_recursively(e->get_prev_edge()->get_symmetric(), visit);
        traverse_triangles_recursively(e->get_next_edge()->get_symmetric(), visit);
    }

public:
    template<class _visit>
    void traverse_triangles(_visit visit)
    {
        traverse_triangles_recursively(_edge_range.left, visit);
    }
    void reset_traverse();
};

__ariel_end__

#endif
