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

#include <gslib/error.h>
#include <gslib/utility.h>
#include <ariel/delaunay.h>

__ariel_begin__

static const float dt_tolerance = 1e-6f;
static const double dt_tolerance64 = 1e-12;

static double dt_area(const vec2& a, const vec2& b, const vec2& c) { return ((double)b.x - a.x) * ((double)c.y - a.y) - ((double)b.y - a.y) * ((double)c.x - a.x); }
static double dt_lengthsq(const vec2& q) { return (double)q.x * q.x + (double)q.y * q.y; }
static bool dt_ccw(const vec2& a, const vec2& b, const vec2& c) { return dt_area(a, b, c) > 0.0; }
static bool dt_left_of(const vec2& p, dt_edge* e) { return dt_ccw(p, e->get_org_point(), e->get_dest_point()); }
static bool dt_right_of(const vec2& p, dt_edge* e) { return dt_ccw(p, e->get_dest_point(), e->get_org_point()); }
static bool dt_valid(dt_edge* e, dt_edge* basel) { return dt_right_of(e->get_dest_point(), basel); }

static bool dt_in_circle(const vec2& a, const vec2& b, const vec2& c, const vec2& d)
{
    if(&d == &a || &d == &b || &d == &c)
        return false;
    double f = dt_lengthsq(a) * dt_area(b, c, d) -
        dt_lengthsq(b) * dt_area(a, c, d) +
        dt_lengthsq(c) * dt_area(a, b, d) -
        dt_lengthsq(d) * dt_area(a, b, c);
    return f > dt_tolerance64;
}

static void dt_splice(dt_edge* e1, dt_edge* e2)
{
    assert(e1 && e2);
    e1->get_prev_edge()->set_next_edge(e2);
    e2->get_prev_edge()->set_next_edge(e1);
    auto* t1 = e1->get_prev_edge();
    auto* t2 = e2->get_prev_edge();
    e1->set_prev_edge(t2);
    e2->set_prev_edge(t1);
}

static void dt_reset_edge_checked(dt_edges& edges)
{
    for(auto* e : edges) {
        assert(e);
        e->set_checked(false);
        e->get_symmetric()->set_checked(false);
    }
}

static bool dt_on_edge(const vec2& p, const vec2& p1, const vec2& p2)
{
    auto t1 = vec2().sub(p, p1).lengthsq();
    auto t2 = vec2().sub(p, p2).lengthsq();
    if(t1 < dt_tolerance || t2 < dt_tolerance)
        return true;
    auto t3 = vec2().sub(p1, p2).lengthsq();
    if(t1 > t3 || t2 > t3)
        return false;
    vec3 coef;
    get_linear_coefficient(coef, p1, vec2().sub(p2, p1));
    auto f = vec3dot(&coef, &vec3(p.x, p.y, 1.f));
    return abs(f) < dt_tolerance;
}

static bool dt_on_edge(const vec2& p, dt_edge* e)
{
    assert(e);
    return dt_on_edge(p, e->get_org_point(), e->get_dest_point());
}

static dt_edge* dt_locate_point(const vec2& p, dt_edge* start)
{
    assert(start);
    auto* e = start;
    int s = 0;
    for(;;) {
        if(p == e->get_org_point() || p == e->get_dest_point())
            return e;
        else if(!dt_right_of(p, e->get_org_next())) {
            if(s != 1) {
                s = 1;
                start = e;
            }
            e = e->get_org_next();
            if(e == start)
                return nullptr;
        }
        else if(!dt_right_of(p, e->get_dest_prev())) {
            if(s != 2) {
                s = 2;
                start = e;
            }
            e = e->get_dest_prev();
            if(e == start)
                return nullptr;
        }
    }
    return nullptr;
}

static dt_edge* dt_locate_joint(dt_joint* j, dt_edge* start)
{
    assert(start);
    auto* b = j->get_binding();
    auto& p = j->get_point();
    auto* e = start;
    int s = 0;
    for(;;) {
        if(b == e->get_org()->get_binding() || b == e->get_dest()->get_binding())
            return e;
        else if(dt_on_edge(p, e))
            return e;
        else if(!dt_right_of(p, e->get_org_next())) {
            if(s != 1) {
                s = 1;
                start = e;
            }
            e = e->get_org_next();
            if(e == start)
                return nullptr;
        }
        else if(!dt_right_of(p, e->get_dest_prev())) {
            if(s != 2) {
                s = 2;
                start = e;
            }
            e = e->get_dest_prev();
            if(e == start)
                return nullptr;
        }
        else
            return e;
    }
    return nullptr;
}

static bool dt_line_intersect(const vec2& p1, const vec2& p2, const vec2& p3, const vec2& p4)
{
    if(p1 == p3 || p1 == p4 || p2 == p3 || p2 == p4)
        return false;
    return (dt_ccw(p3, p4, p1) != dt_ccw(p3, p4, p2)) &&
        (dt_ccw(p1, p2, p3) != dt_ccw(p1, p2, p4));
}

static void dt_check_edge_linkage(dt_edge* e)
{
    assert(e);
    auto* e1 = e->get_prev_edge();
    auto* e2 = e->get_next_edge();
    assert(e1->get_next_edge() == e);
    assert(e1->get_dest() == e->get_org());
    assert(e2->get_prev_edge() == e);
    assert(e->get_dest() == e2->get_org());
}

static void dt_trace_edge(dt_edge* e)
{
    assert(e);
    auto& p1 = e->get_org_point();
    auto& p2 = e->get_dest_point();
    trace(_t("@moveTo %f, %f;\n"), p1.x, p1.y);
    trace(_t("@lineTo %f, %f;\n"), p2.x, p2.y);
    dt_check_edge_linkage(e);
    dt_check_edge_linkage(e->get_symmetric());
}

static void dt_trace_edge_mel(dt_edge* e)
{
    assert(e);
    const vec2& p1 = e->get_org_point();
    const vec2& p2 = e->get_dest_point();
    trace(_t("curve -d 1 -p %f %f 0 -p %f %f 0;\n"), p1.x, p1.y, p2.x, p2.y);
}

void dt_collect_intersect_edges_c(const vec2& p1, const vec2& p2, dt_edge_list& edges, dt_edge* last);

static void dt_collect_intersect_edges_w(const vec2& p1, const vec2& p2, dt_edge_list& edges, dt_edge* init)
{
    assert(init);
    /* which means the collection was over. */
    if(init->get_org_point() == p2)
        return;
    /*
     * overkill:
     * if the init edge was overlap the p1 - p2 linked line, then we should skip it.
     * considered that the following detections were done from the org prev to itself,
     * so hereby we continue this procedure by its left next.
     */
    if(dt_on_edge(init->get_dest_point(), p1, p2))
        return dt_collect_intersect_edges_w(p1, p2, edges, init->get_left_next());
    dt_edge* ints = nullptr;
    auto* e = init->get_left_next();
    if(dt_line_intersect(p1, p2, e->get_org_point(), e->get_dest_point()))
        ints = e;
    else {
        for(auto* dir = init->get_org_prev(); dir != init; dir = dir->get_org_prev()) {
            /* repeat the overkill */
            if(dt_on_edge(dir->get_dest_point(), p1, p2))
                return dt_collect_intersect_edges_w(p1, p2, edges, dir->get_left_next());
            e = dir->get_left_next();
            if(dt_line_intersect(p1, p2, e->get_org_point(), e->get_dest_point())) {
                ints = e;
                break;
            }
        }
    }
    if(!ints)
        return;
    assert(dt_right_of(p2, ints));
    ints = ints->get_symmetric();
    edges.push_back(ints);
    dt_collect_intersect_edges_c(p1, p2, edges, ints);
}

static void dt_collect_intersect_edges_c(const vec2& p1, const vec2& p2, dt_edge_list& edges, dt_edge* last)
{
    assert(last);
    auto* e1 = last->get_left_prev();
    auto* e2 = last->get_left_next();
    assert(e1 && e2);
    auto& p = e1->get_org_point();
    assert(p == e2->get_dest_point() && "should all be triangles.");
    if(p == p2)
        return;
    if(dt_on_edge(p, p1, p2))
        return dt_collect_intersect_edges_w(p1, p2, edges, e1);
    auto* ints = dt_line_intersect(p1, p2, e1->get_org_point(), e1->get_dest_point()) ? e1 : e2;
    assert(dt_line_intersect(p1, p2, ints->get_org_point(), ints->get_dest_point()));
    assert(dt_right_of(p2, ints));
    ints = ints->get_symmetric();
    edges.push_back(ints);
    dt_collect_intersect_edges_c(p1, p2, edges, ints);
}

static void dt_collect_intersect_edges(const vec2& p1, const vec2& p2, dt_edge_list& edges, dt_edge* start)
{
    assert(start);
    auto* init = dt_locate_point(p1, start);
    if(!init)
        return;
    if(init->get_org_point() != p1)
        init = init->get_symmetric();
    assert(init->get_org_point() == p1 &&
        "This function only used for the situation that the query point was on the mesh."
        );
    dt_collect_intersect_edges_w(p1, p2, edges, init);
}

static bool dt_can_flip(dt_edge* e)
{
    assert(e);
    auto* e1 = e->get_left_next();
    auto* e2 = e->get_right_next();
    auto& p1 = e->get_org_point();
    auto& p2 = e2->get_org_point();
    auto& p3 = e1->get_org_point();
    auto& p4 = e1->get_dest_point();
    bool b1 = dt_ccw(p1, p2, p3),
        b2 = dt_ccw(p2, p3, p4),
        b3 = dt_ccw(p3, p4, p1),
        b4 = dt_ccw(p4, p1, p2);
    return b1 == b2 && b2 == b3 && b3 == b4;
}

static bool dt_need_flip(dt_edge* e)
{
    assert(e);
    auto* e1 = e->get_left_next();
    auto* e2 = e->get_right_next();
    auto& p1 = e->get_org_point();
    auto& p2 = e2->get_org_point();
    auto& p3 = e1->get_org_point();
    auto& p4 = e1->get_dest_point();
    bool b1 = dt_ccw(p1, p2, p3),
        b2 = dt_ccw(p2, p3, p4),
        b3 = dt_ccw(p3, p4, p1),
        b4 = dt_ccw(p4, p1, p2);
    if(!(b1 == b2 && b2 == b3 && b3 == b4))
        return false;
    float d1 = vec2().sub(p1, p3).lengthsq(),
        d2 = vec2().sub(p2, p4).lengthsq();
    return d1 > d2;
}

static void dt_flip(dt_edge* e)
{
    assert(e);
    auto* a = e->get_org_prev();
    auto* b = e->get_symmetric()->get_org_prev();
    dt_splice(e, a);
    dt_splice(e->get_symmetric(), b);
    dt_splice(e, a->get_left_next());
    dt_splice(e->get_symmetric(), b->get_left_next());
    e->set_org(a->get_dest());
    e->set_dest(b->get_dest());
}

static void dt_trace_edges(dt_edge* e)
{
    assert(e);
    if(e->is_checked())
        return;
    stack<dt_edge*> to_be_traced;
    dt_trace_edge(e);
    e->set_checked(true);
    to_be_traced.push(e->get_symmetric());
    for(auto* n = e->get_next_edge(); n != e; n = n->get_next_edge()) {
        if(!n->is_checked()) {
            dt_trace_edge(n);
            n->set_checked(true);
        }
        to_be_traced.push(n->get_symmetric());
    }
    while(!to_be_traced.empty()) {
        dt_trace_edges(to_be_traced.top());
        to_be_traced.pop();
    }
}

static void dt_trace_edge_info(dt_edge* e)
{
    assert(e);
    trace(_t("#trace edge:\n"));
    dt_trace_edge(e);
    auto* symm = e->get_symmetric();
    assert(symm);
    trace(_t("#trace symmetric:\n"));
    dt_trace_edge(symm);
    auto* prev = e->get_prev_edge();
    auto* next = e->get_next_edge();
    assert(prev && next);
    trace(_t("#prev:\n"));
    dt_trace_edge(prev);
    trace(_t("#next: \n"));
    dt_trace_edge(next);
}

static void dt_trace_edge_loop(dt_edge* e)
{
    assert(e);
    dt_trace_edge(e);
    for(auto* n = e->get_next_edge(); n != e; n = n->get_next_edge())
        dt_trace_edge(n);
}

static bool dt_joint_compare(const dt_joint* i, const dt_joint* j) 
{
    assert(i && j);
    auto& p1 = i->get_point();
    auto& p2 = j->get_point();
    if(p1.x < p2.x)
        return true;
    else if(p1.x > p2.x)
        return false;
    return p1.y < p2.y;
};

dt_edge::dt_edge()
{
    _org = nullptr;
    _prev = _next = _symmetric = nullptr;
    _constraint = false;
    _boundary = false;
    _checked = false;
}

bool dt_edge::is_outside_boundary() const
{
    assert(_prev && _next);
    auto* i = _prev->get_org();
    auto* j = _next->get_dest();
    assert(i && j);
    /*
     * There is an exception:
     * if the whole triangulation has only 1 triangle, this test will fail.
     */
    return i != j;
}

bool dt_edge::is_boundary_by_dcel() const
{
    assert(_symmetric);
    if(!is_outside_boundary()) {
        if(!_symmetric->is_outside_boundary()) {
            /* need detection */
            auto is_cw = [](const dt_edge* e)->bool {
                assert(e);
                auto& p1 = e->get_org_point();
                auto& p2 = e->get_dest_point();
                auto& p3 = e->get_prev_edge()->get_org_point();
                return is_concave_angle(p1, p2, p3);
            };
            bool cw1 = is_cw(this);
            bool cw2 = is_cw(_symmetric);
            return cw1 != cw2;
        }
        return true;
    }
    else if(!_symmetric->is_outside_boundary())
        return true;
    return false;
}

void delaunay_triangulation::initialize(dt_input_joints& inputs)
{
    dt_joint_ptrs pre_sort;
    for(auto& p : inputs)
        pre_sort.push_back(&p);
    pre_sort.sort(dt_joint_compare);
    /* delete same points, maybe a problem. */
    if(!pre_sort.empty()) {
        auto i = pre_sort.begin(), j = std::next(i), end = pre_sort.end();
        while(j != end) {
            if((*i)->get_point() == (*j)->get_point()) {
                pre_sort.erase(j);
                j = std::next(i);
                continue;
            }
            i = j ++;
        }
    }
    for(auto* p : pre_sort)
        _sorted_joints.push_back(p);
}

void delaunay_triangulation::run()
{
    int c = (int)_sorted_joints.size();
    if(c == 0)
        return;
    _edge_range = delaunay(0, c - 1);
}

void delaunay_triangulation::clear()
{
    _sorted_joints.clear();
    _edge_range.left = _edge_range.right = nullptr;
    for(auto* p : _edge_holdings) {
        delete p->get_symmetric();
        delete p;
    }
    _edge_holdings.clear();
}

dt_edge* delaunay_triangulation::add_constraint(const vec2& p1, const vec2& p2)
{
    auto* init = dt_locate_point(p1, _edge_range.left);
    if(!init) {
        init = dt_locate_point(p1, _edge_range.right);
        if(!init)
            return nullptr;
    }
    assert(init);
    if(init->get_org_point() != p1)
        init = init->get_symmetric();
    assert(init->get_org_point() == p1);
    /* test if the edge exists. */
    dt_edge* exist_edge = nullptr;
    if(init->get_dest_point() == p2)
        exist_edge = init;
    else {
        for(auto* e = init->get_org_prev(); e != init; e = e->get_org_prev()) {
            if(e->get_dest_point() == p2) {
                exist_edge = e;
                break;
            }
        }
    }
    if(exist_edge) {
        /* set the edge as constraint whatever */
        exist_edge->set_constraint(true);
        exist_edge->get_symmetric()->set_constraint(true);
        return exist_edge;
    }
    /* otherwise find all the intersected edges */
    dt_edge_list edges;
    dt_collect_intersect_edges_w(p1, p2, edges, init);
    if(edges.empty())
        return nullptr;
    /* find the head tail of the loop */
    auto* firstcut = edges.front();
    auto* cand1 = firstcut->get_left_prev();
    auto* cand2 = firstcut->get_right_next();
    auto* loop1 = (cand1->get_org_point() == p1) ? cand1 : cand2;
    assert(loop1->get_org_point() == p1);
    auto* lastcut = edges.back();
    auto* cand3 = lastcut->get_left_next();
    auto* cand4 = lastcut->get_right_prev();
    auto* loop2 = (cand3->get_dest_point() == p2) ? cand3 : cand4;
    assert(loop2->get_dest_point() == p2);
    for(auto* e : edges) {
        assert(!e->is_constraint());
        destroy_edge_pair(e);
    }
    assert(loop1->get_symmetric()->is_outside_boundary());
    assert(loop2->is_outside_boundary());
    /* cut the space by constraint */
    auto* cut = connect_edges(loop1->get_symmetric(), loop2->get_next_edge());
    assert(cut);
    /* triangulate the two parts of the cut by shrink */
    shrink_triangulate(cut);
    shrink_triangulate(cut->get_symmetric());
    /* tag the constraint */
    cut->set_constraint(true);
    cut->get_symmetric()->set_constraint(true);
    return cut;
}

void delaunay_triangulation::trim(dt_edge_list& edges)
{
    dt_edge_list for_trim;
    for(auto* e : edges) {
        assert(e && e->is_constraint());
        e->set_boundary(true);
        if(e->is_boundary_by_dcel())
            continue;
        auto* e1 = e->get_prev_edge();
        auto* e2 = e->get_next_edge();
        assert(e1 && e2);
        if(!e1->is_checked())
            collect_trim_edges(for_trim, e1->get_symmetric());
        if(!e2->is_checked())
            collect_trim_edges(for_trim, e2->get_symmetric());
    }
    bool need_reset_left = false, need_reset_right = false;
    auto* rleft = _edge_range.left;
    auto* rright = _edge_range.right;
    for(auto* e : for_trim) {
        if((e == rleft) || (e->get_symmetric() == rleft))
            need_reset_left = true;
        if((e == rright) || (e->get_symmetric() == rright))
            need_reset_right = true;
        destroy_edge_pair(e);
    }
    auto f = _edge_holdings.begin();
    auto pr = [](dt_edge* e)-> bool { return e->is_boundary() || e->is_boundary_by_dcel(); };
    if(need_reset_left) {
        f = std::find_if(f, _edge_holdings.end(), pr);
        if(f != _edge_holdings.end())
            set_range_left(*f);
    }
    if(need_reset_right) {
        f = std::find_if(f, _edge_holdings.end(), pr);
        if(f != _edge_holdings.end())
            set_range_right(*f);
    }
}

void delaunay_triangulation::tracing() const
{
    for(auto* e : _edge_holdings) {
        assert(e);
        dt_trace_edge(e);
    }
}

void delaunay_triangulation::trace_heuristically() const
{
    auto* e = _edge_range.left;
    dt_trace_edges(e);
    dt_reset_edge_checked(
        const_cast<dt_edges&>(_edge_holdings)
        );
    return;     /* no verbose. */
    trace(_t("trace edge infos.\n"));
    for(auto* p : _edge_holdings) {
        dt_trace_edge_info(p);
        dt_trace_edge_info(p->get_symmetric());
    }
}

void delaunay_triangulation::trace_mel() const
{
    for(auto e : _edge_holdings) {
        assert(e);
        dt_trace_edge_mel(e);
    }
}

bool delaunay_triangulation::is_in_range(int begin, int end, dt_joint* joint)
{
    if(!joint)
        return false;
    const auto& min_joint = _sorted_joints[begin];
    if(dt_joint_compare(joint, min_joint))
        return false;
    const auto& max_joint = _sorted_joints[end];
    return !dt_joint_compare(max_joint, joint);
}

dt_edge_range delaunay_triangulation::delaunay(int begin, int end)
{
    int size = end - begin + 1;
    dt_edge_range ret;
    if(size == 2) {
        auto* e = create_edge_pair();
        e->set_org(_sorted_joints.at(begin));
        e->set_dest(_sorted_joints.at(end));
        ret.left = e;
        ret.right = e->get_symmetric();
        return ret;
    }
    else if(size == 3) {
        auto* e1 = create_edge_pair();
        auto* e2 = create_edge_pair();
        assert(e1 && e2);
        dt_splice(e1->get_symmetric(), e2);
        auto* p1 = _sorted_joints.at(begin);
        auto* p2 = _sorted_joints.at(begin + 1);
        auto* p3 = _sorted_joints.at(end);
        e1->set_org(p1);
        e1->set_dest(p2);
        e2->set_org(p2);
        e2->set_dest(p3);
        dt_edge* e3;
        if(dt_ccw(p1->get_point(), p2->get_point(), p3->get_point())) {
            e3 = connect_edges(e2, e1);
            ret.left = e1;
            ret.right = e2->get_symmetric();
            return ret;
        }
        else if(dt_ccw(p1->get_point(), p3->get_point(), p2->get_point())) {
            e3 = connect_edges(e2, e1);
            ret.left = e3->get_symmetric();
            ret.right = e3;
            return ret;
        }
        else {
            ret.left = e1;
            ret.right = e2->get_symmetric();
            return ret;
        }
    }
    else {
        int center = begin + (size / 2);
        auto left_range = delaunay(begin, center - 1);
        auto right_range = delaunay(center, end);
        auto* ldo = left_range.left;
        auto* ldi = left_range.right;
        auto* rdi = right_range.left;
        auto* rdo = right_range.right;
        for(;;) {
            if(dt_left_of(rdi->get_org_point(), ldi))
                ldi = ldi->get_left_next();
            else if(dt_right_of(ldi->get_org_point(), rdi))
                rdi = rdi->get_right_prev();
            else
                break;
        }
        auto* basel = connect_edges(rdi->get_symmetric(), ldi);
        if(ldi->get_org_point() == ldo->get_org_point())
            ldo = basel->get_symmetric();
        if(rdi->get_org_point() == rdo->get_org_point())
            rdo = basel;
        for(;;) {
            auto* lcand = basel->get_symmetric()->get_org_next();
            if(dt_valid(lcand, basel)) {
                while(is_in_range(begin, center-1, lcand->get_org_next()->get_dest()) && // 删除左侧边要保证在左侧集合中，不做此判断会导致 basel 被误删 
                    dt_right_of(lcand->get_org_next()->get_dest_point(), basel) && // 候选边要保证与 basel 夹角小于 180 度，即在 basel 的右侧
                    dt_in_circle(basel->get_dest_point(), basel->get_org_point(),
                    lcand->get_dest_point(), lcand->get_org_next()->get_dest_point()
                    )) {
                    auto* t = lcand->get_org_next();
                    destroy_edge_pair(lcand);
                    lcand = t;
                }
            }
            auto* rcand = basel->get_org_prev();
            if(dt_valid(rcand, basel)) {
                while(is_in_range(center, end, rcand->get_org_prev()->get_dest()) && // 删除右侧边要保证在右侧集合中，不做此判断会导致 basel 被误删 
                    dt_right_of(rcand->get_org_prev()->get_dest_point(), basel) && // 候选边要保证与 basel 夹角小于 180 度，即在 basel 的右侧
                    dt_in_circle(basel->get_dest_point(), basel->get_org_point(),
                    rcand->get_dest_point(), rcand->get_org_prev()->get_dest_point()
                    )) {
                    auto* t = rcand->get_org_prev();
                    destroy_edge_pair(rcand);
                    rcand = t;
                }
            }
            if(!dt_valid(lcand, basel) && !dt_valid(rcand, basel))
                break;
            basel = (!dt_valid(lcand, basel) ||
                (dt_valid(rcand, basel) && dt_in_circle(lcand->get_dest_point(), lcand->get_org_point(), rcand->get_org_point(), rcand->get_dest_point()))
                ) ?
                connect_edges(rcand, basel->get_symmetric()) :
                connect_edges(basel->get_symmetric(), lcand->get_symmetric());
        }
        ret.left = ldo;
        ret.right = rdo;
        return ret;
    }
}

dt_edge* delaunay_triangulation::create_edge_pair()
{
    auto* e1 = new dt_edge;
    auto* e2 = new dt_edge;
    e1->set_symmetric(e2);
    e2->set_symmetric(e1);
    e1->set_prev_edge(e2);
    e1->set_next_edge(e2);
    e2->set_prev_edge(e1);
    e2->set_next_edge(e1);
    _edge_holdings.insert(e1);
    return e1;
}

dt_edge* delaunay_triangulation::connect_edges(dt_edge* e1, dt_edge* e2)
{
    assert(e1 && e2);
    auto* e = create_edge_pair();
    assert(e);
    dt_splice(e, e1->get_left_next());
    dt_splice(e->get_symmetric(), e2);
    e->set_org(e1->get_dest());
    e->set_dest(e2->get_org());
    return e;
}

void delaunay_triangulation::destroy_edge_pair(dt_edge* e)
{
    assert(e);
    auto* esymm = e->get_symmetric();
    dt_splice(e, e->get_org_prev());
    dt_splice(esymm, esymm->get_org_prev());
    auto f = _edge_holdings.find(e);
    if(f != _edge_holdings.end())
        _edge_holdings.erase(f);
    else
        _edge_holdings.erase(esymm);
    delete esymm;
    delete e;
}

void delaunay_triangulation::shrink_triangulate(dt_edge* cut)
{
    assert(cut);
    auto* from = cut->get_next_edge();
    auto* to = cut->get_prev_edge();
    assert(from && to);
    /* if the cut was already a triangle */
    if(from->get_dest() == to->get_org())
        return;
    dt_edge_list strips;
    for(auto* e = from; e != cut; e = e->get_next_edge())
        strips.push_back(e);
    /* do shrink */
    dt_edge_list temps;
    shrink_recursively(strips, temps);
    /* to optimize the triangulation, we choose to do a check & flip process */
    for(auto* e : temps) {
        if(dt_need_flip(e))
            dt_flip(e);
    }
}

void delaunay_triangulation::shrink_recursively(dt_edge_list& strips, dt_edge_list& temps)
{
    assert(strips.size() > 2);
    dt_edge_list nextstrips;
    int i = 0, j = 1;
    int cap = (int)strips.size();
    while(i < cap) {
        if(j == cap) {
            nextstrips.push_back(strips.at(i));
            break;
        }
        auto* e1 = strips.at(i);
        auto* e2 = strips.at(j);
        assert(e1 && e2);
        if(dt_ccw(e1->get_org_point(), e2->get_org_point(), e2->get_dest_point())) {
            auto* e = connect_edges(e2, e1);
            assert(e);
            nextstrips.push_back(e->get_symmetric());
            temps.push_back(e);
            i = j + 1;
            j = i + 1;
        }
        else {
            nextstrips.push_back(e1);
            i = j ++;
        }
    }
    if(nextstrips.size() <= 2)
        return;
    shrink_recursively(nextstrips, temps);
}

void delaunay_triangulation::collect_trim_edges(dt_edge_list& edges, dt_edge* e)
{
    assert(e && !e->is_checked());
    if(e->is_constraint())
        return;
    e->set_checked(true);
    e->get_symmetric()->set_checked(true);
    edges.push_back(e);
    if(e->is_boundary())
        return;
    auto* e1 = e->get_next_edge();
    auto* e2 = e->get_prev_edge();
    assert(e1 && e2);
    if(!e1->is_checked())
        collect_trim_edges(edges, e1->get_symmetric());
    if(!e2->is_checked())
        collect_trim_edges(edges, e2->get_symmetric());
}

void delaunay_triangulation::collect_triangles(dt_traversal_triangles& triangles)
{
    dt_traversal_triangles bound_triangles;    /* try to filter triangle holes */
    int s = (int)triangles.size();
    traverse_triangles([&bound_triangles, &triangles](void* b1, void* b2, void* b3, bool b[3]) {
        dt_traversal_triangle t;
        t.binding1 = b1;
        t.binding2 = b2;
        t.binding3 = b3;
        t.is_boundary[0] = b[0];
        t.is_boundary[1] = b[1];
        t.is_boundary[2] = b[2];
        (b[0] && b[1] && b[2]) ? bound_triangles.emplace_back(t) : triangles.emplace_back(t);
    });
    if((int)triangles.size() <= s) {    /* means == */
        if(!bound_triangles.empty())
            triangles.emplace_back(bound_triangles.front());
    }
}

void delaunay_triangulation::reset_traverse()
{
    dt_reset_edge_checked(_edge_holdings);
}

__ariel_end__
