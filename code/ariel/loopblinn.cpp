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

#include <gslib/error.h>
#include <pink/utility.h>
#include <ariel/loopblinn.h>

__ariel_begin__

lb_line* lb_get_span(lb_line_list& span, lb_line* start)
{
    assert(start);
    span.push_back(start);
    auto* joint1 = start->get_next_joint();
    assert(joint1);
    if(joint1->get_type() != lbt_control_joint)
        return joint1->get_next_line();
    auto* next1 = joint1->get_next_line();
    assert(next1);
    span.push_back(next1);
    auto* joint2 = next1->get_next_joint();
    assert(joint2);
    if(joint2->get_type() != lbt_control_joint)
        return joint2->get_next_line();
    auto* next2 = joint2->get_next_line();
    assert(next2);
    span.push_back(next2);
    auto* joint3 = next2->get_next_joint();
    assert(joint3);
    assert(joint3->get_type() != lbt_control_joint);
    return joint3->get_next_line();
}

static lb_line* lb_walk_span(lb_line* start)
{
    assert(start);
    auto* joint1 = start->get_next_joint();
    assert(joint1);
    if(joint1->get_type() != lbt_control_joint)
        return joint1->get_next_line();
    auto* next1 = joint1->get_next_line();
    assert(next1);
    auto* joint2 = next1->get_next_joint();
    assert(joint2);
    if(joint2->get_type() != lbt_control_joint)
        return joint2->get_next_line();
    auto* next2 = joint2->get_next_line();
    assert(next2);
    auto* joint3 = next2->get_next_joint();
    assert(joint3);
    assert(joint3->get_type() != lbt_control_joint);
    return joint3->get_next_line();
}

static lb_line* lb_trace_span(lb_line* line)
{
    assert(line);
    lb_line_list span;
    line = lb_get_span(span, line);
    int size = (int)span.size();
    if(size == 1) {
        auto* line1 = span.at(0);
        auto& p1 = line1->get_next_point();
        trace(_t("@lineTo %f, %f;\n"), p1.x, p1.y);
    }
    else if(size == 2) {
        auto* line2 = span.at(1);
        auto& p1 = line2->get_prev_point();
        auto& p2 = line2->get_next_point();
        trace(_t("@quadTo %f, %f, %f, %f;\n"), p1.x, p1.y, p2.x, p2.y);
    }
    else if(size == 3) {
        auto* line2 = span.at(1);
        auto* line3 = span.at(2);
        auto& p1 = line2->get_prev_point();
        auto& p2 = line3->get_prev_point();
        auto& p3 = line3->get_next_point();
        trace(_t("@cubicTo %f, %f, %f, %f, %f, %f;\n"), p1.x, p1.y, p2.x, p2.y, p3.x, p3.y);
    }
    else {
        assert(!"unexpected.");
    }
    return line;
}

static lb_line* lb_trace_span_segment(lb_line* line)
{
    assert(line);
    auto& p = line->get_prev_point();
    trace(_t("@moveTo %f, %f;\n"), p.x, p.y);
    return lb_trace_span(line);
}

static void lb_connect(lb_joint* joint, lb_line* line)
{
    assert(joint && line);
    joint->set_next_line(line);
    line->set_prev_joint(joint);
}

static void lb_connect(lb_line* line, lb_joint* joint)
{
    assert(joint && line);
    line->set_next_joint(joint);
    joint->set_prev_line(line);
}

static void lb_connect(lb_joint* joint1, lb_line* line, lb_joint* joint2)
{
    assert(joint1 && line && joint2);
    lb_connect(joint1, line);
    lb_connect(line, joint2);
}

static void lb_connect(lb_bisp_line* line1, lb_bisp_line* line2)
{
    assert(line1 && line2);
    line1->set_next(line2);
    line2->set_prev(line1);
}

static void lb_collect_end_joints(lb_joint_list& joints, lb_line* start)
{
    assert(start);
    joints.push_back(start->get_prev_joint());
    auto* line = lb_walk_span(start);
    for(; line != start; line = lb_walk_span(line))
        joints.push_back(line->get_prev_joint());
}

static bool lb_is_clockwise(lb_line* start)
{
    assert(start);
    lb_joint_list eps;
    lb_collect_end_joints(eps, start);
    assert(eps.size() > 0);
    int size = (int)eps.size();
    if(size == 1 || size == 2) {
        auto* d = eps.at(0);
        assert(d);
        auto* j1 = d->get_prev_joint();
        auto* j2 = d->get_next_joint();
        assert(j1 && j2);
        return !pink::is_concave_angle(j1->get_point(), d->get_point(), j2->get_point());
    }
    /* find right most node. */
    auto* d = eps.at(0);
    for(int i = 1; i < size; i ++) {
        auto* p = eps.at(i);
        if(p->get_point().x > d->get_point().x)
            d = p;
    }
    assert(d);
    auto* j1 = d->get_prev_joint();
    auto* j2 = d->get_next_joint();
    assert(j1 && j2);
    return !pink::is_concave_angle(j1->get_point(), d->get_point(), j2->get_point());
}

template<class _line>
static bool lb_is_convex(_line* line1, _line* line2)
{
    assert(line1 && line2);
    assert(line1->get_next_point() == line2->get_prev_point());
    auto& p1 = line1->get_prev_point();
    auto& p2 = line2->get_prev_point();
    auto& p3 = line2->get_next_point();
    return !pink::is_concave_angle(p1, p2, p3);
}

/*
 * The reason why we have to sample a point which inside the polygon rather than just take
 * an arbitrarily point on the boundary of the polygon was that, the path may have coincide
 * points on both the boundary and the holes.
 * The sample process was simple, just find a convex angle of the path, take the clockwise
 * logic into consideration, then retrieve the center point of this convex angle.
 */

void lb_get_current_span(lb_line_list& span, lb_control_joint* joint)
{
    assert(joint);
    auto* prevj = joint->get_prev_joint();
    assert(prevj);
    if(prevj->get_type() == lbt_end_joint) {
        auto* nextj = joint->get_next_joint();
        assert(nextj);
        if(nextj->get_type() == lbt_end_joint) {
            span.push_back(prevj->get_next_line());
            span.push_back(joint->get_next_line());
            return;
        }
        else {
            assert(nextj->get_type() == lbt_control_joint);
            auto* nextnextj = nextj->get_next_joint();
            assert(nextnextj);
            assert(nextnextj->get_type() == lbt_end_joint);
            span.push_back(prevj->get_next_line());
            span.push_back(joint->get_next_line());
            span.push_back(nextj->get_next_line());
            return;
        }
    }
    else {
        assert(prevj->get_type() == lbt_control_joint);
        auto* prevprevj = prevj->get_prev_joint();
        assert(prevprevj);
        assert(prevprevj->get_type() == lbt_end_joint);
        span.push_back(prevprevj->get_next_line());
        span.push_back(prevj->get_next_line());
        span.push_back(joint->get_next_line());
        return;
    }
}

static void lb_front_sample_on_boundary(vec2& p, lb_end_joint* joint)
{
    assert(joint);
    auto* nextj = joint->get_next_joint();
    assert(nextj);
    if(nextj->get_type() == lbt_end_joint) {
        auto& p1 = joint->get_point();
        auto& p2 = nextj->get_point();
        p.add(p1, p2);
        p.scale(0.5f);
        return;
    }
    lb_line_list span;
    lb_get_span(span, joint->get_next_line());
    int size = (int)span.size();
    assert(size == 2 || size == 3);
    if(size == 2) {
        auto* line1 = span.at(0);
        auto* line2 = span.at(1);
        vec3 para[2];
        pink::get_quad_parameter_equation(para, line1->get_prev_point(), line1->get_next_point(), line2->get_next_point());
        pink::eval_quad(p, para, 0.1f);
    }
    else {
        auto* line1 = span.at(0);
        auto* line2 = span.at(1);
        auto* line3 = span.at(2);
        auto& c1 = line1->get_prev_point();
        auto& c2 = line2->get_prev_point();
        auto& c3 = line3->get_prev_point();
        auto& c4 = line3->get_next_point();
        float t[2];
        int c = pink::get_cubic_inflection(t, c1, c2, c3, c4);
        float s = (c == 0) ? 0.1f : t[0] * 0.5f;
        vec4 para[2];
        pink::get_cubic_parameter_equation(para, c1, c2, c3, c4);
        pink::eval_cubic(p, para, s);
    }
}

static void lb_back_sample_on_boundary(vec2& p, lb_end_joint* joint)
{
    assert(joint);
    auto* prevj = joint->get_prev_joint();
    assert(prevj);
    if(prevj->get_type() == lbt_end_joint) {
        auto& p1 = prevj->get_point();
        auto& p2 = joint->get_point();
        p.add(p1, p2);
        p.scale(0.5f);
        return;
    }
    lb_line_list span;
    lb_get_current_span(span, static_cast<lb_control_joint*>(prevj));
    int size = (int)span.size();
    assert(size == 2 || size == 3);
    if(size == 2) {
        auto* line1 = span.at(0);
        auto* line2 = span.at(1);
        vec3 para[2];
        pink::get_quad_parameter_equation(para, line1->get_prev_point(), line1->get_next_point(), line2->get_next_point());
        pink::eval_quad(p, para, 0.9f);
    }
    else {
        auto* line1 = span.at(0);
        auto* line2 = span.at(1);
        auto* line3 = span.at(2);
        auto& c1 = line1->get_prev_point();
        auto& c2 = line2->get_prev_point();
        auto& c3 = line3->get_prev_point();
        auto& c4 = line3->get_next_point();
        float t[2];
        int c = pink::get_cubic_inflection(t, c1, c2, c3, c4);
        float s = (c == 0) ? 0.9f : 1.f - (1.f - t[c - 1]) * 0.5f;
        vec4 para[2];
        pink::get_cubic_parameter_equation(para, c1, c2, c3, c4);
        pink::eval_cubic(p, para, s);
    }
}

static void lb_get_sample_in_range(vec2& p, const vec2& c1, const vec2& c2, const vec2& c3, float t1, float t2)
{
    vec3 para[2];
    pink::get_quad_parameter_equation(para, c1, c2, c3);
    assert(t1 < t2);
    float elapse = (t2 - t1) / 3.f;
    float t = t1 + elapse;
    float s = t2 - elapse;
    vec2 p1, p2;
    pink::eval_quad(p1, para, t);
    pink::eval_quad(p2, para, s);
    p.add(p1, p2);
    p.scale(0.5f);
}

static void lb_get_sample_in_range(vec2& p, const vec2& c1, const vec2& c2, const vec2& c3, const vec2& c4, float t1, float t2)
{
    vec4 para[2];
    pink::get_cubic_parameter_equation(para, c1, c2, c3, c4);
    assert(t1 < t2);
    float elapse = (t2 - t1) / 3.f;
    float t = t1 + elapse;
    float s = t2 - elapse;
    vec2 p1, p2;
    pink::eval_cubic(p1, para, t);
    pink::eval_cubic(p2, para, s);
    p.add(p1, p2);
    p.scale(0.5f);
}

static void lb_curve_sample(vec2& p, lb_control_joint* joint)
{
    assert(joint);
    lb_line_list span;
    lb_get_current_span(span, joint);
    int size = (int)span.size();
    assert(size == 2 || size == 3);
    if(size == 2) {
        auto* line1 = span.at(0);
        auto* line2 = span.at(1);
        lb_get_sample_in_range(p, line1->get_prev_point(), line2->get_prev_point(), line2->get_next_point(), 0.f, 1.f);
        return;
    }
    else {
        assert(size == 3);
        auto* line1 = span.at(0);
        auto* line2 = span.at(1);
        auto* line3 = span.at(2);
        auto& c1 = line1->get_prev_point();
        auto& c2 = line2->get_prev_point();
        auto& c3 = line3->get_prev_point();
        auto& c4 = line3->get_next_point();
        float t[2];
        int c = pink::get_cubic_inflection(t, c1, c2, c3, c4);
        if(c == 1) {    /* only 1 inflection matters */
            bool is_prev_ctl = joint->get_prev_joint()->get_type() == lbt_end_joint;
            return is_prev_ctl ? lb_get_sample_in_range(p, c1, c2, c3, c4, 0.f, t[0]) :
                lb_get_sample_in_range(p, c1, c2, c3, c4, t[0], 1.f);
        }
        lb_get_sample_in_range(p, c1, c2, c3, c4, 0.f, 1.f);
        return;
    }
}

static void lb_calc_sample_cw(vec2& p, lb_line* line1, lb_line* line2)
{
    assert(line1 && line2 && line1->get_next_line() == line2);
    auto* joint1 = line1->get_prev_joint();
    auto* joint2 = line2->get_prev_joint();
    auto* joint3 = line2->get_next_joint();
    auto tag1 = joint1->get_type();
    auto tag2 = joint2->get_type();
    auto tag3 = joint3->get_type();
    if(tag2 == lbt_end_joint) {
        if((tag1 == lbt_end_joint) && (tag3 == lbt_end_joint)) {
            auto& p1 = joint1->get_point();
            auto& p2 = joint2->get_point();
            auto& p3 = joint3->get_point();
            vec2 tot;
            tot.add(p1, p2);
            tot += p3;
            p.scale(tot, 0.3333333f);
            return;
        }
        vec2 p1, p2;
        lb_front_sample_on_boundary(p1, static_cast<lb_end_joint*>(joint2));
        lb_back_sample_on_boundary(p2, static_cast<lb_end_joint*>(joint2));
        p.add(p1, p2);
        p.scale(0.5f);
        return;
    }
    else {
        assert(tag2 == lbt_control_joint);
        lb_curve_sample(p, static_cast<lb_control_joint*>(joint2));
        return;
    }
}

static bool lb_get_sample_cw(vec2& p, lb_line* start)
{
    assert(start);
    auto* next = start->get_next_line();
    if(next == start)
        return false;
    if(lb_is_convex(start, next)) {
        lb_calc_sample_cw(p, start, next);
        return true;
    }
    auto* curr = next;
    while(curr != start) {
        next = curr->get_next_line();
        if(lb_is_convex(curr, next)) {
            lb_calc_sample_cw(p, curr, next);
            return true;
        }
        curr = next;
    }
    return false;
}

static void lb_calc_sample_ccw(vec2& p, lb_line* line1, lb_line* line2)
{
    assert(line1 && line2 && line1->get_next_line() == line2);
    auto* joint1 = line1->get_prev_joint();
    auto* joint2 = line2->get_prev_joint();
    auto* joint3 = line2->get_next_joint();
    auto tag1 = joint1->get_type();
    auto tag2 = joint2->get_type();
    auto tag3 = joint3->get_type();
    if(tag2 == lbt_end_joint) {
        if((tag1 == lbt_end_joint) && (tag3 == lbt_end_joint)) {
            auto& p1 = joint1->get_point();
            auto& p2 = joint2->get_point();
            auto& p3 = joint3->get_point();
            vec2 tot;
            tot.add(p1, p2);
            tot += p3;
            p.scale(tot, 0.3333333f);
            return;
        }
        vec2 p1, p2;
        lb_front_sample_on_boundary(p1, static_cast<lb_end_joint*>(joint2));
        lb_back_sample_on_boundary(p2, static_cast<lb_end_joint*>(joint2));
        p.add(p1, p2);
        p.scale(0.5f);
        return;
    }
    else {
        assert(tag2 == lbt_control_joint);
        lb_curve_sample(p, static_cast<lb_control_joint*>(joint2));
        return;
    }
}

static bool lb_get_sample_ccw(vec2& p, lb_line* start)
{
    assert(start);
    auto* next = start->get_next_line();
    if(next == start)
        return false;
    if(!lb_is_convex(start, next)) {
        lb_calc_sample_ccw(p, start, next);
        return true;
    }
    auto* curr = next;
    while(curr != start) {
        next = curr->get_next_line();
        if(!lb_is_convex(curr, next)) {
            lb_calc_sample_ccw(p, curr, next);
            return true;
        }
        curr = next;
    }
    return false;
}

class lb_linear_span:
    public lb_span
{
protected:
    lb_line*            _line;

public:
    lb_linear_span(lb_line* l) { setup(l); }
    lb_span_type get_type() const override { return lst_linear; }
    bool can_split() const override { return false; }
    bool is_overlapped(const lb_span* span) const override;
    lb_line* get_line() const { return _line; }
    void setup(lb_line* l)
    {
        assert(l);
        _line = l;
        auto& p1 = l->get_prev_point();
        auto& p2 = l->get_next_point();
        _rc.left = gs_min(p1.x, p2.x);
        _rc.top = gs_min(p1.y, p2.y);
        _rc.right = gs_max(p1.x, p2.x);
        _rc.bottom = gs_max(p1.y, p2.y);
    }
};

class lb_quad_span:
    public lb_span
{
protected:
    lb_line*            _lines[2];

public:
    lb_quad_span(lb_line* l1, lb_line* l2) { setup(l1, l2); }
    lb_span_type get_type() const override { return lst_quad; }
    bool can_split() const override { return true; }
    bool is_overlapped(const lb_span* span) const override;
    lb_line* get_line(int i) const { return _lines[i]; }
    void setup(lb_line* l1, lb_line* l2)
    {
        assert(l1 && l2);
        _lines[0] = l1, _lines[1] = l2;
        auto& p1 = l1->get_prev_point();
        auto& p2 = l2->get_prev_point();
        auto& p3 = l2->get_next_point();
        _rc.left = gs_min(gs_min(p1.x, p2.x), p3.x);
        _rc.top = gs_min(gs_min(p1.y, p2.y), p3.y);
        _rc.right = gs_max(gs_max(p1.x, p2.x), p3.x);
        _rc.bottom = gs_max(gs_max(p1.y, p2.y), p3.y);
    }
};

class lb_cubic_span:
    public lb_span
{
protected:
    lb_line*            _lines[3];

public:
    lb_cubic_span(lb_line* l1, lb_line* l2, lb_line* l3) { setup(l1, l2, l3); }
    lb_span_type get_type() const override { return lst_cubic; }
    bool can_split() const override { return true; }
    bool is_overlapped(const lb_span* span) const override;
    lb_line* get_line(int i) const { return _lines[i]; }
    void setup(lb_line* l1, lb_line* l2, lb_line* l3)
    {
        assert(l1 && l2 && l3);
        _lines[0] = l1, _lines[1] = l2, _lines[2] = l3;
        auto& p1 = l1->get_prev_point();
        auto& p2 = l2->get_prev_point();
        auto& p3 = l3->get_prev_point();
        auto& p4 = l3->get_next_point();
        _rc.left = gs_min(gs_min(gs_min(p1.x, p2.x), p3.x), p4.x);
        _rc.top = gs_min(gs_min(gs_min(p1.y, p2.y), p3.y), p4.y);
        _rc.right = gs_max(gs_max(gs_max(p1.x, p2.x), p3.x), p4.x);
        _rc.bottom = gs_max(gs_max(gs_max(p1.y, p2.y), p3.y), p4.y);
    }
};

static bool lb_is_line_intersected(lb_line* line1, lb_line* line2)
{
    assert(line1 && line2);
    vec2 p, d1, d2;
    auto& p1 = line1->get_prev_point();
    auto& p2 = line1->get_next_point();
    auto& p3 = line2->get_prev_point();
    auto& p4 = line2->get_next_point();
    d1.sub(p2, p1);
    d2.sub(p4, p3);
    if(d1 == vec2(0.f, 0.f) || d2 == vec2(0.f, 0.f))
        return false;
    pink::intersectp_linear_linear(p, p1, p3, d1, d2);
    float t = pink::linear_reparameterize(p1, p2, p);
    if(t < 0.f || t > 1.f)
        return false;
    float s = pink::linear_reparameterize(p3, p4, p);
    return s >= 0.f && s <= 1.f;
}

static bool lb_is_span_overlapped(const lb_linear_span* span1, const lb_quad_span* span2)
{
    assert(span1 && span2);
    auto* line1 = span1->get_line();
    auto* line2 = span2->get_line(0);
    auto* line3 = span2->get_line(1);
    assert(line1 && line2 && line3);
    return lb_is_line_intersected(line1, line2) ||
        lb_is_line_intersected(line1, line3);
}

static bool lb_is_span_overlapped(const lb_linear_span* span1, const lb_cubic_span* span2)
{
    assert(span1 && span2);
    auto* line1 = span1->get_line();
    auto* line2 = span2->get_line(0);
    auto* line3 = span2->get_line(1);
    auto* line4 = span2->get_line(2);
    assert(line1 && line2 && line3 && line4);
    return lb_is_line_intersected(line1, line2) ||
        lb_is_line_intersected(line1, line3) ||
        lb_is_line_intersected(line1, line4);
}

static bool lb_is_span_overlapped(const lb_quad_span* span1, const lb_quad_span* span2)
{
    assert(span1 && span2);
    auto* line1 = span1->get_line(0);
    auto* line2 = span1->get_line(1);
    auto* line3 = span2->get_line(0);
    auto* line4 = span2->get_line(1);
    assert(line1 && line2 && line3 && line4);
    return lb_is_line_intersected(line1, line3) ||
        lb_is_line_intersected(line1, line4) ||
        lb_is_line_intersected(line2, line3) ||
        lb_is_line_intersected(line2, line4);
}

static bool lb_is_span_overlapped(const lb_quad_span* span1, const lb_cubic_span* span2)
{
    assert(span1 && span2);
    auto* line1 = span1->get_line(0);
    auto* line2 = span1->get_line(1);
    auto* line3 = span2->get_line(0);
    auto* line4 = span2->get_line(1);
    auto* line5 = span2->get_line(2);
    assert(line1 && line2 && line3 && line4 && line5);
    return lb_is_line_intersected(line1, line3) ||
        lb_is_line_intersected(line1, line4) ||
        lb_is_line_intersected(line1, line5) ||
        lb_is_line_intersected(line2, line3) ||
        lb_is_line_intersected(line2, line4) ||
        lb_is_line_intersected(line2, line5);
}

static bool lb_is_span_overlapped(const lb_cubic_span* span1, const lb_cubic_span* span2)
{
    assert(span1 && span2);
    auto* line1 = span1->get_line(0);
    auto* line2 = span1->get_line(1);
    auto* line3 = span1->get_line(2);
    auto* line4 = span2->get_line(0);
    auto* line5 = span2->get_line(1);
    auto* line6 = span2->get_line(2);
    assert(line1 && line2 && line3 && line4 && line5 && line6);
    return lb_is_line_intersected(line1, line4) ||
        lb_is_line_intersected(line1, line5) ||
        lb_is_line_intersected(line1, line6) ||
        lb_is_line_intersected(line2, line4) ||
        lb_is_line_intersected(line2, line5) ||
        lb_is_line_intersected(line2, line6) ||
        lb_is_line_intersected(line3, line4) ||
        lb_is_line_intersected(line3, line5) ||
        lb_is_line_intersected(line3, line6);
}

bool lb_linear_span::is_overlapped(const lb_span* span) const
{
    assert(span);
    switch(span->get_type())
    {
    case lst_linear:
        /* linear - linear will never be overlapped */
        return false;
    case lst_quad:
        return lb_is_span_overlapped(this, static_cast<const lb_quad_span*>(span));
    case lst_cubic:
        return lb_is_span_overlapped(this, static_cast<const lb_cubic_span*>(span));
    }
    assert(!"unexpected.");
    return false;
}

bool lb_quad_span::is_overlapped(const lb_span* span) const
{
    assert(span);
    switch(span->get_type())
    {
    case lst_linear:
        return lb_is_span_overlapped(static_cast<const lb_linear_span*>(span), this);
    case lst_quad:
        return lb_is_span_overlapped(this, static_cast<const lb_quad_span*>(span));
    case lst_cubic:
        return lb_is_span_overlapped(this, static_cast<const lb_cubic_span*>(span));;
    }
    assert(!"unexpected.");
    return false;
}

bool lb_cubic_span::is_overlapped(const lb_span* span) const
{
    assert(span);
    switch(span->get_type())
    {
    case lst_linear:
        return lb_is_span_overlapped(static_cast<const lb_linear_span*>(span), this);
    case lst_quad:
        return lb_is_span_overlapped(static_cast<const lb_quad_span*>(span), this);
    case lst_cubic:
        return lb_is_span_overlapped(this, static_cast<const lb_cubic_span*>(span));;
    }
    assert(!"unexpected.");
    return false;
}

static lb_line* lb_create_span(lb_span_list& spans, lb_line* line)
{
    assert(line);
    lb_line_list span;
    auto* next = lb_get_span(span, line);
    switch(span.size())
    {
    case 1:
        spans.push_back(gs_new(lb_linear_span, span.at(0)));
        break;
    case 2:
        spans.push_back(gs_new(lb_quad_span, span.at(0), span.at(1)));
        break;
    case 3:
        spans.push_back(gs_new(lb_cubic_span, span.at(0), span.at(1), span.at(2)));
        break;
    default:
        assert(!"unexpected size of span.");
        return 0;
    }
    return next;
}

static void lb_create_spans(lb_span_list& spans, lb_line* start)
{
    assert(start);
    auto* line = lb_create_span(spans, start);
    while(line != start)
        line = lb_create_span(spans, line);
}

static void lb_convert_ncoord(lb_line* start, const mat3& m)
{
    assert(start);
    auto* first = start->get_prev_joint();
    auto* joint = start->get_next_joint();
    assert(first && joint);
    auto cvt = [&m](lb_joint* j) {
        assert(j);
        auto& p = j->get_point();
        j->set_ncoord_point(vec2().transformcoord(p, m));
    };
    cvt(first);
    for(; joint != first; joint = joint->get_next_joint())
        cvt(joint);
}

lb_joint* lb_joint::get_prev_joint() const
{
    assert(_prev && (_prev->get_next_joint() == this));
    return _prev->get_prev_joint();
}

lb_joint* lb_joint::get_next_joint() const
{
    assert(_next && (_next->get_prev_joint() == this));
    return _next->get_next_joint();
}

bool lb_end_joint::prev_is_curve() const
{
    assert(_prev);
    auto* prevj = _prev->get_prev_joint();
    assert(prevj);
    return prevj->get_type() == lbt_control_joint;
}

bool lb_end_joint::next_is_curve() const
{
    assert(_next);
    auto* nextj = _next->get_next_joint();
    assert(nextj);
    return nextj->get_type() == lbt_control_joint;
}

lb_line* lb_line::get_prev_line() const
{
    assert(_joint[0]);
    assert(_joint[0]->get_next_line() == this);
    return _joint[0]->get_prev_line();
}

lb_line* lb_line::get_next_line() const
{
    assert(_joint[1]);
    assert(_joint[1]->get_prev_line() == this);
    return _joint[1]->get_next_line();
}

lb_line* lb_line::get_line_between(lb_joint* j1, lb_joint* j2)
{
    if(j1->get_prev_joint() == j2) {
        assert(j2->get_next_joint() == j1);
        return j1->get_prev_line();
    }
    else if(j1->get_next_joint() == j2) {
        assert(j2->get_prev_joint() == j1);
        return j1->get_next_line();
    }
    return 0;
}

lb_bisp_line::lb_bisp_line()
{
    _prev = _next = 0;
    _bin[0] = _bin[1] = 0;
    _le.x = NAN;
}

bool lb_bisp_line::is_le_available() const
{
    return !isnan(_le.x);
}

void lb_bisp_line::calc_linear_expression()
{
    if(is_le_available())
        return;
    auto& p1 = get_prev_point();
    auto& p2 = get_next_point();
    pink::get_linear_coefficient(_le, p1, vec2().sub(p2, p1));
}

float lb_bisp_line::get_le_dot(const vec2& p) const
{
    assert(is_le_available());
    return _le.dot(vec3(p.x, p.y, 1.f));
}

void lb_bisp_line::tracing() const
{
    assert(_next);
    auto& p1 = get_prev_point();
    auto& p2 = get_next_point();
    trace(_t("@moveTo %f, %f;\n"), p1.x, p1.y);
    trace(_t("@lineTo %f, %f;\n"), p2.x, p2.y);
    if(_bin[0])
        _bin[0]->tracing();
    if(_bin[1])
        _bin[1]->tracing();
}

lb_bisp::~lb_bisp()
{
    for(auto* p : _lines) { gs_del(lb_bisp_line, p); }
    _lines.clear();
    _center = 0;
}

void lb_bisp::setup(lb_line* start)
{
    assert(start);
    auto* first = create(start);
    assert(first);
    if(first->get_prev()->get_prev() == first->get_next()) {
        /* already a triangle. */
        _center = first;
        return;
    }
    _center = shrink(first);
}

bool lb_bisp::is_inside(const vec2& p) const
{
    assert(_center);
    auto* c = query(p);
    return !(c && c->is_boundary());
}

void lb_bisp::tracing() const
{
    if(!_center)
        return;
    auto* line1 = _center->get_prev();
    auto* line2 = _center->get_next();
    assert(line1 && line2);
    assert((line1 == line2) ||
        (line1->get_prev() == line2 && line2->get_next() == line1)
        );
    _center->tracing();
    line1->tracing();
    line2->tracing();
}

void lb_bisp::trace_loop() const
{
    assert(_center);
    _center->tracing();
    for(auto* line = _center->get_next(); line != _center; line = line->get_next())
        line->tracing();
}

lb_bisp_line* lb_bisp::create_line()
{
    auto* p = gs_new(lb_bisp_line);
    assert(p);
    _lines.push_back(p);
    return p;
}

lb_bisp_line* lb_bisp::create(lb_line* start)
{
    assert(start);
    lb_line_list span;
    auto* line = lb_get_span(span, start);
    lb_bisp_line* seg[2];
    create_segment(span, seg);
    if(line == start) {
        lb_connect(seg[1], seg[0]);
        return seg[0];
    }
    auto* first = seg[0];
    while(line != start) {
        span.clear();
        line = lb_get_span(span, line);
        auto* last = seg[1];
        create_segment(span, seg);
        lb_connect(last, seg[0]);
    }
    lb_connect(seg[1], first);
    /* calc linear expressions */
    first->calc_linear_expression();
    for(auto* line = first->get_next(); line != first; line = line->get_next())
        line->calc_linear_expression();
    return first;
}

void lb_bisp::create_segment(lb_line_list& span, lb_bisp_line* seg[2])
{
    int size = (int)span.size();
    switch(size)
    {
    case 1:
        return create_linear_segment(span.at(0), seg);
    case 2:
        return create_quadratic_segment(span.at(0), span.at(1), seg);
    case 3:
        return create_cubic_segment(span.at(0), span.at(1), span.at(2), seg);
    default:
        assert(!"unexpected span.");
        return;
    }
}

void lb_bisp::create_linear_segment(lb_line* line1, lb_bisp_line* seg[2])
{
    assert(line1);
    auto& p1 = line1->get_prev_point();
    auto* s = create_line();
    s->set_prev_point(p1);
    seg[0] = seg[1] = s;
}

void lb_bisp::create_quadratic_segment(lb_line* line1, lb_line* line2, lb_bisp_line* seg[2])
{
    assert(line1 && line2);
    auto& p1 = line1->get_prev_point();
    auto& p2 = line2->get_prev_point();
    auto& p3 = line2->get_next_point();
    auto* first = create_line();
    first->set_prev_point(p1);
    auto* last = first;
    vec3 para[2];
    pink::get_quad_parameter_equation(para, p1, p2, p3);
    int step = pink::get_rough_interpolate_step(p1, p2, p3);
    float t, chord;
    t = chord = 1.f / (step - 1);
    for(int i = 1; i < step - 1; i ++, t += chord) {
        vec2 p;
        pink::eval_quad(p, para, t);
        auto* line = create_line();
        line->set_prev_point(p);
        lb_connect(last, line);
        last = line;
    }
    seg[0] = first;
    seg[1] = last;
}

void lb_bisp::create_cubic_segment(lb_line* line1, lb_line* line2, lb_line* line3, lb_bisp_line* seg[2])
{
    assert(line1 && line2 && line3);
    auto& p1 = line1->get_prev_point();
    auto& p2 = line2->get_prev_point();
    auto& p3 = line3->get_prev_point();
    auto& p4 = line3->get_next_point();
    auto* first = create_line();
    first->set_prev_point(p1);
    auto* last = first;
    vec4 para[2];
    pink::get_cubic_parameter_equation(para, p1, p2, p3, p4);
    int step = pink::get_rough_interpolate_step(p1, p2, p3, p4);
    float t, chord;
    t = chord = 1.f / (step - 1);
    for(int i = 1; i < step - 1; i ++, t += chord) {
        vec2 p;
        pink::eval_cubic(p, para, t);
        auto* line = create_line();
        line->set_prev_point(p);
        lb_connect(last, line);
        last = line;
    }
    seg[0] = first;
    seg[1] = last;
}

lb_bisp_line* lb_bisp::shrink(lb_bisp_line* start)
{
    assert(start);
    int ctr = 1;
    lb_bisp_line *first, *last, *curr, *next = start->get_next();
    if(lb_is_convex(start, next)) {
        first = last = make_shrink(start, next);
        curr = next->get_next();
    }
    else {
        first = last = start;
        curr = next;
    }
    assert(curr);
    while(curr != start) {
        next = curr->get_next();
        assert(next);
        if(next == start) {
            if((first == start) && lb_is_convex(curr, start)) {
                auto* line = make_shrink(curr, start);      /* could do an extra shrink */
                lb_connect(line, first->get_next());
                line->calc_linear_expression();
                first = line;
            }
            else {
                lb_connect(last, curr);
                last->calc_linear_expression();
                last = curr;
                ctr ++;
            }
            break;
        }
        if(lb_is_convex(curr, next)) {
            auto* line = make_shrink(curr, next);
            lb_connect(last, line);
            last->calc_linear_expression();
            last = line;
            curr = next->get_next();
        }
        else {
            lb_connect(last, curr);
            last->calc_linear_expression();
            last = curr;
            curr = next;
        }
        ctr ++;
    }
    assert(first && last);
    lb_connect(last, first);
    last->calc_linear_expression();
    return ctr <= 3 ? first : shrink(first);
}

lb_bisp_line* lb_bisp::make_shrink(lb_bisp_line* line1, lb_bisp_line* line2)
{
    assert(line1 && line2);
    auto* line = create_line();
    line->set_binary(0, line1);
    line->set_binary(1, line2);
    line->set_prev_point(line1->get_prev_point());
    return line;
}

lb_bisp_line* lb_bisp::query(const vec2& p) const
{
    assert(_center);
    /* the center could be binary or triangle. */
    auto* line1 = _center->get_prev();
    auto* line2 = _center->get_next();
    assert(line1 && line2);
    if(line1 == line2) {
        float d = _center->get_le_dot(p);
        if(pink::fuzzy_zero(d))
            return _center;
        return d < 0 ? query(p, _center) : query(p, line1);
    }
    else {
        assert(line1->get_prev() == line2);
        float d1 = _center->get_le_dot(p);
        if(pink::fuzzy_zero(d1))
            return _center;
        if(d1 < 0)
            return query(p, _center);
        float d2 = line1->get_le_dot(p);
        if(pink::fuzzy_zero(d2))
            return line1;
        if(d2 < 0)
            return query(p, line1);
        float d3 = line2->get_le_dot(p);
        if(pink::fuzzy_zero(d3))
            return line2;
        if(d3 < 0)
            return line2;
        return 0;
    }
}

lb_bisp_line* lb_bisp::query(const vec2& p, lb_bisp_line* last) const
{
    assert(last);
    if(last->is_boundary())
        return last;
    auto* line1 = last->get_binary(0);
    auto* line2 = last->get_binary(1);
    assert(line1 && line2);
    float d1 = line1->get_le_dot(p);
    if(pink::fuzzy_zero(d1))
        return line1;
    if(d1 < 0)
        return query(p, line1);
    float d2 = line2->get_le_dot(p);
    if(pink::fuzzy_zero(d2))
        return line2;
    if(d2 < 0)
        return query(p, line2);
    return last;
}

void lb_polygon::convert_to_ncoord(const mat3& m)
{
    assert(_boundary);
    lb_convert_ncoord(_boundary, m);
    for(auto* p : _holes)
        lb_convert_ncoord(p, m);
}

void lb_polygon::create_dt_joints()
{
    auto collect_joints = [this](lb_line* start) {
        assert(start);
        auto* first = start->get_prev_joint();
        auto* joint = start->get_next_joint();
        assert(first && joint);
        _dtjoints.push_back(dt_joint(first->get_point(), first));
        for(; joint != first; joint = joint->get_next_joint())
            _dtjoints.push_back(dt_joint(joint->get_point(), joint));
    };
    collect_joints(_boundary);
    for(auto* p : _holes)
        collect_joints(p);
}

void lb_polygon::pack_constraints()
{
    dt_edge_list bound;
    list<vec2> packpt;
    auto classify_quad = [&](lb_line_list& span) {
        assert(span.size() == 2);
        auto* line1 = span.at(0);
        auto* line2 = span.at(1);
        auto& p1 = line1->get_prev_point();
        auto& p2 = line2->get_prev_point();
        auto& p3 = line2->get_next_point();
        if(!pink::is_concave_angle(p1, p2, p3)) {
            auto* e1 = _cdt.add_constraint(p1, p2);
            auto* e2 = _cdt.add_constraint(p2, p3);
            assert(e1 && e2);
            bound.push_back(e1);
            bound.push_back(e2);
            packpt.push_back(p1);
            packpt.push_back(p3);
        }
        else {
            auto* e1 = _cdt.add_constraint(p1, p3);
            assert(e1);
            bound.push_back(e1);
            packpt.push_back(p1);
            packpt.push_back(p2);
            packpt.push_back(p2);
            packpt.push_back(p3);
        }
    };
    auto classify_cubic = [&](lb_line_list& span) {
        assert(span.size() == 3);
        auto* line1 = span.at(0);
        auto* line2 = span.at(1);
        auto* line3 = span.at(2);
        auto& p1 = line1->get_prev_point();
        auto& p2 = line2->get_prev_point();
        auto& p3 = line3->get_prev_point();
        auto& p4 = line3->get_next_point();
        float t[2];
        int c = pink::get_cubic_inflection(t, p1, p2, p3, p4);
        if(c == 0) {
            if(!pink::is_concave_angle(p1, p2, p4)) {
                auto* e1 = _cdt.add_constraint(p1, p2);
                auto* e2 = _cdt.add_constraint(p2, p3);
                auto* e3 = _cdt.add_constraint(p3, p4);
                assert(e1 && e2 && e3);
                bound.push_back(e1);
                bound.push_back(e2);
                bound.push_back(e3);
                packpt.push_back(p1);
                packpt.push_back(p4);
            }
            else {
                auto* e1 = _cdt.add_constraint(p1, p4);
                assert(e1);
                bound.push_back(e1);
                packpt.push_back(p1);
                packpt.push_back(p2);
                packpt.push_back(p2);
                packpt.push_back(p3);
                packpt.push_back(p3);
                packpt.push_back(p4);
            }
        }
        else if(c == 1) {
            if(!pink::is_concave_angle(p1, p2, p3)) {
                auto* e1 = _cdt.add_constraint(p1, p2);
                auto* e2 = _cdt.add_constraint(p2, p4);
                assert(e1 && e2);
                bound.push_back(e1);
                bound.push_back(e2);
                packpt.push_back(p1);
                packpt.push_back(p3);
                packpt.push_back(p3);
                packpt.push_back(p4);
            }
            else {
                auto* e1 = _cdt.add_constraint(p1, p3);
                auto* e2 = _cdt.add_constraint(p3, p4);
                assert(e1 && e2);
                bound.push_back(e1);
                bound.push_back(e2);
                packpt.push_back(p1);
                packpt.push_back(p2);
                packpt.push_back(p2);
                packpt.push_back(p4);
            }
        }
        else {
            assert(c == 2);
            if(!pink::is_concave_angle(p1, p2, p4)) {
                auto* e1 = _cdt.add_constraint(p1, p3);
                auto* e2 = _cdt.add_constraint(p3, p2);
                auto* e3 = _cdt.add_constraint(p2, p4);
                assert(e1 && e2 && e3);
                bound.push_back(e1);
                bound.push_back(e2);
                bound.push_back(e3);
                packpt.push_back(p1);
                packpt.push_back(p4);
            }
            else {
                auto* e1 = _cdt.add_constraint(p1, p4);
                assert(e1);
                bound.push_back(e1);
                packpt.push_back(p1);
                packpt.push_back(p3);
                packpt.push_back(p3);
                packpt.push_back(p2);
                packpt.push_back(p2);
                packpt.push_back(p4);
            }
        }
    };
    auto classify = [&](lb_line_list& span) {
        switch(span.size())
        {
        case 1:
            {
                auto* line = span.at(0);
                auto* e = _cdt.add_constraint(line->get_prev_point(), line->get_next_point());
                bound.push_back(e);
                break;
            }
        case 2:
            classify_quad(span);
            break;
        case 3:
            classify_cubic(span);
            break;
        default:
            assert(!"unexpected.");
            return;
        }
    };
    auto classfify_around = [&](lb_line* start) {
        assert(start);
        lb_line_list span;
        auto* line = lb_get_span(span, start);
        classify(span);
        while(line != start) {
            span.clear();
            line = lb_get_span(span, line);
            classify(span);
        }
    };
    assert(_boundary);
    classfify_around(_boundary);
    for(auto* p : _holes)
        classfify_around(p);
    assert(!bound.empty());
    _cdt.trim(bound);
    _cdt.set_range_left(bound.front());
    if(packpt.empty())
        return;
    assert(packpt.size() % 2 == 0);
    auto i = packpt.begin(), j = std::next(i), end = packpt.end();
    for(;;) {
        _cdt.add_constraint(*i, *j);
        i = ++ j;
        if(i == end)
            break;
        ++ j;
        assert(j != end);
    }
}

void lb_polygon::build_cdt()
{
    create_dt_joints();
    _cdt.initialize(_dtjoints);
    _cdt.run();
    pack_constraints();
}

void lb_polygon::tracing() const
{
    trace_boundary();
    trace_holes();
}

void lb_polygon::trace_boundary() const
{
    assert(_boundary);
    auto& p = _boundary->get_prev_point();
    trace(_t("@moveTo %f, %f;\n"), p.x, p.y);
    auto* line = lb_trace_span(_boundary);
    while(line != _boundary)
        line = lb_trace_span(line);
}

void lb_polygon::trace_last_hole() const
{
    assert(!_holes.empty());
    auto* start = _holes.back();
    auto& p = start->get_prev_point();
    trace(_t("@moveTo %f, %f;\n"), p.x, p.y);
    auto* line = lb_trace_span(start);
    while(line != start)
        line = lb_trace_span(line);
}

void lb_polygon::trace_holes() const
{
    for(auto* start : _holes) {
        assert(start);
        auto& p = start->get_prev_point();
        trace(_t("@moveTo %f, %f;\n"), p.x, p.y);
        auto* line = lb_trace_span(start);
        while(line != start)
            line = lb_trace_span(line);
    }
}

loop_blinn_processor::~loop_blinn_processor()
{
    for(auto* p : _line_holdings) { gs_del(lb_line, p); }
    for(auto* p : _joint_holdings) { gs_del(lb_joint, p); }
    for(auto* p : _polygons) { gs_del(lb_polygon, p); }
    for(auto* p : _span_holdings) { gs_del(lb_span, p); }
    _line_holdings.clear();
    _joint_holdings.clear();
    _polygons.clear();
    _span_holdings.clear();
}

void loop_blinn_processor::proceed(const painter_path& path)
{
    hierarchy_flatten(path);
    if(_polygons.empty())
        return;
    for(auto* p : _polygons) {
        check_boundary(p);
        check_holes(p);
        check_rtree(p);
    }
    calc_klm_coords();
    for(auto* p : _polygons)
        p->build_cdt();
}

void loop_blinn_processor::trace_polygons() const
{
    trace(_t("#trace polygons start:\n"));
    trace(_t("@!\n"));
    for(auto* p : _polygons)
        p->tracing();
    trace(_t("@@\n"));
}

void loop_blinn_processor::trace_bisps() const
{
    trace(_t("#trace bisps start:\n"));
    trace(_t("@!\n"));
    for(auto* p : _polygons)
        p->trace_bisp();
    trace(_t("@@\n"));
}

void loop_blinn_processor::trace_rtree() const
{
    trace(_t("#trace rtree start:\n"));
    for(auto* p : _polygons)
        p->trace_rtree();
}

lb_polygon* loop_blinn_processor::create_polygon()
{
    auto* p = gs_new(lb_polygon);
    assert(p);
    _polygons.push_back(p);
    return p;
}

lb_line* loop_blinn_processor::create_line()
{
    auto* p = gs_new(lb_line);
    assert(p);
    _line_holdings.push_back(p);
    return p;
}

template<class _joint>
lb_joint* loop_blinn_processor::create_joint(const vec2& p)
{
    auto* j = gs_new(_joint);
    assert(j);
    j->set_point(p);
    _joint_holdings.push_back(j);
    return static_cast<lb_joint*>(j);
}

void loop_blinn_processor::hierarchy_flatten(const painter_path& path)
{
    int size = path.size();
    assert(size > 0);
    lb_line* line = 0;
    int next = create_patch(line, path, 0);
    assert(line);
    assert(lb_is_clockwise(line));
    auto* poly = create_polygon();
    poly->set_boundary(line);
    assert(next <= size);
    if(next == size)
        return;
    lb_polygon_stack st;
    do { next = hierarchy_flatten(path, next, poly, st); }
    while(next < size);
}

int loop_blinn_processor::hierarchy_flatten(const painter_path& path, int start, lb_polygon* parent, lb_polygon_stack& st)
{
    assert(parent);
    int size = path.size();
    assert(start < size);
    lb_line* line = 0;
    int next = create_patch(line, path, start);
    bool cw = lb_is_clockwise(line);
    if(cw) {
        auto* poly = create_polygon();
        poly->set_boundary(line);
        assert(next <= size);
        if(next == size)
            return size;
        vec2 sample;
        lb_get_sample_cw(sample, line);
        parent->ensure_create_bisp();
        bool in = parent->is_inside(sample);
        if(in)
            st.push(parent);
        return hierarchy_flatten(path, next, poly, st);
    }
    else {
        vec2 sample;
        lb_get_sample_ccw(sample, line);
        parent->ensure_create_bisp();
        bool in = parent->is_inside(sample);
        if(in) {
            parent->add_hole(line);
            return (next == size) ? size :
                hierarchy_flatten(path, next, parent, st);
        }
        else {
            /* find a suitable boundary */
            while(!st.empty()) {
                auto* uptrace = st.top();
                st.pop();
                uptrace->ensure_create_bisp();
                in = uptrace->is_inside(sample);
                if(in) {
                    uptrace->add_hole(line);
                    return (next == size) ? size :
                        hierarchy_flatten(path, next, uptrace, st);
                }
            }
            assert(!"unexpected path.");
            return size;
        }
    }
}

int loop_blinn_processor::create_patch(lb_line*& line, const painter_path& path, int start)
{
    assert(start < path.size());
    auto* node0 = path.get_node(start);
    assert(node0 && (node0->get_tag() == painter_path::pt_moveto));
    int size = path.size();
    assert(size > 0);
    auto* first = create_joint<lb_end_joint>(node0->get_point());
    auto* prev = first;
    int i = start + 1;
    for(; i < size; i ++) {
        auto* node = path.get_node(i);
        assert(node);
        if(node->get_tag() == painter_path::pt_moveto)
            break;
        prev = create_segment(prev, path, i);
    }
    assert(first && prev);
    if(first->get_point() == prev->get_point()) {
        auto* line = prev->get_prev_line();
        assert(line);
        lb_connect(line, first);
    }
    else {
        auto* line = create_line();
        lb_connect(prev, line, first);
        line->set_opened(true);
    }
    line = first->get_next_line();
    return i;
}

lb_joint* loop_blinn_processor::create_segment(lb_joint* prev, const painter_path& path, int i)
{
    assert(prev);
    assert(i < path.size());
    auto* node = path.get_node(i);
    assert(node);
    auto t = node->get_tag();
    switch(t)
    {
    case painter_path::pt_lineto:
        {
            auto* line = create_line();
            auto* joint = create_joint<lb_end_joint>(node->get_point());
            lb_connect(prev, line, joint);
            return joint;
        }
    case painter_path::pt_quadto:
        {
            auto* qnode = static_cast<const painter_path::quad_to_node*>(node);
            auto* line1 = create_line();
            auto* line2 = create_line();
            auto* joint1 = create_joint<lb_control_joint>(qnode->get_control());
            auto* joint2 = create_joint<lb_end_joint>(qnode->get_point());
            lb_connect(prev, line1, joint1);
            lb_connect(joint1, line2, joint2);
            return joint2;
        }
    case painter_path::pt_cubicto:
        {
            auto* cnode = static_cast<const painter_path::cubic_to_node*>(node);
            auto* line1 = create_line();
            auto* line2 = create_line();
            auto* line3 = create_line();
            auto* joint1 = create_joint<lb_control_joint>(cnode->get_control1());
            auto* joint2 = create_joint<lb_control_joint>(cnode->get_control2());
            auto* joint3 = create_joint<lb_end_joint>(cnode->get_point());
            lb_connect(prev, line1, joint1);
            lb_connect(joint1, line2, joint2);
            lb_connect(joint2, line3, joint3);
            return joint3;
        }
    }
    assert(!"unexpected.");
    return 0;
}

/*
 * This function would try to avoid the situation that the control path of the
 * boundary runs outside the boundary itself.
 * Such kind of situation would be a problem in the following CDT process.
 * Collect all the concave control points first, then check them by the bisp
 * to see if they were outside the boundary, these checks needn't be too strict,
 * split them and make sure they won't cross the boundary.
 */
void loop_blinn_processor::check_boundary(lb_polygon* poly)
{
    assert(poly);
    auto* start = poly->get_boundary();
    assert(start);
    lb_joint_list concaves;
    auto* firstj = start->get_prev_joint();
    assert(firstj && (firstj->get_type() != lbt_control_joint) &&
        "first joint would always be end joint."
        );
    auto* joint = start->get_next_joint();
    for(; joint != firstj; joint = joint->get_next_joint()) {
        if(joint->get_type() == lbt_control_joint && !lb_is_convex(joint->get_prev_line(), joint->get_next_line()))
            concaves.push_back(joint);
    }
    if(concaves.empty())
        return;
    poly->ensure_create_bisp();
    for(auto* p : concaves)
        check_span(poly, static_cast<lb_control_joint*>(p));
}

void loop_blinn_processor::check_holes(lb_polygon* poly)
{
    /* unlike the boundary, every control points on the hole would have a chance
     * to cross the boundary
     */
    assert(poly);
    auto& holes = poly->get_holes();
    if(holes.empty())
        return;
    lb_joint_list candidates;
    for(auto* holestart : holes) {
        assert(holestart);
        auto* firstj = holestart->get_prev_joint();
        assert(firstj && (firstj->get_type() != lbt_control_joint) &&
            "first joint would always be end joint."
            );
        auto* joint = holestart->get_next_joint();
        for(; joint != firstj; joint = joint->get_next_joint()) {
            if(joint->get_type() == lbt_control_joint)
                candidates.push_back(joint);
        }
    }
    if(candidates.empty())
        return;
    poly->ensure_create_bisp();
    for(auto* p : candidates)
        check_span(poly, static_cast<lb_control_joint*>(p));
}

void loop_blinn_processor::check_span(lb_polygon* poly, lb_control_joint* joint)
{
    assert(poly && joint);
    if(poly->is_inside(joint->get_point()))
        return;
    lb_line_list span;
    lb_get_current_span(span, joint);
    int size = (int)span.size();
    assert(size == 2 || size == 3);
    if(size == 2) {
        lb_joint* sp[5];
        split_quadratic(span.at(0), span.at(1), sp);
        check_span(poly, static_cast<lb_control_joint*>(sp[1]));
        check_span(poly, static_cast<lb_control_joint*>(sp[3]));
    }
    else {
        assert(size == 3);
        lb_joint* sp[7];
        split_cubic(span.at(0), span.at(1), span.at(2), sp, 0.5f);
        check_span(poly, static_cast<lb_control_joint*>(sp[1]));
        check_span(poly, static_cast<lb_control_joint*>(sp[2]));
        check_span(poly, static_cast<lb_control_joint*>(sp[4]));
        check_span(poly, static_cast<lb_control_joint*>(sp[5]));
    }
}

void loop_blinn_processor::check_rtree(lb_polygon* poly)
{
    assert(poly);
    auto& holes = poly->get_holes();
    if(holes.empty())
        return;
    auto* boundary = poly->get_boundary();
    assert(boundary);
    auto& rtr = poly->get_rtree();
    lb_span_list spans;
    lb_create_spans(spans, boundary);
    _span_holdings.insert(_span_holdings.end(), spans.begin(), spans.end());
    for(auto* p : spans)
        rtr.insert(p, p->get_rect());
    for(auto* p : holes) {
        assert(p);
        lb_span_list holespans;
        lb_create_spans(holespans, p);
        _span_holdings.insert(_span_holdings.end(), holespans.begin(), holespans.end());
        for(auto* s : holespans) {
            assert(s);
            (s->get_type() == lst_linear) ? rtr.insert(s, s->get_rect()) :
                check_rtree_span(poly, s);
        }
    }
}

void loop_blinn_processor::check_rtree_span(lb_polygon* poly, lb_span* span)
{
    assert(poly && span);
    assert(span->get_type() != lst_linear);
    auto& rtr = poly->get_rtree();
    list<lb_span*> spans;
    rtr.query(span->get_rect(), spans);
    auto i = spans.begin(), end = spans.end();
    while(i != end) {
        auto* p = *i;
        if(!span->is_overlapped(p)) {
            ++ i;
            continue;
        }
        if(!p->can_split()) {
            split_span_recursively(poly, span);
            return;
        }
        else {
            float area1 = span->get_area();
            float area2 = p->get_area();
            if(area1 > area2) {
                split_span_recursively(poly, span);
                return;
            }
            else {
                rtr.remove(p, p->get_rect());
                auto* s = split_rtree_span(p);
                assert(s);
                rtr.insert(p, p->get_rect());
                rtr.insert(s, s->get_rect());
                bool ints1 = pink::is_rect_intersected(p->get_rect(), span->get_rect());
                bool ints2 = pink::is_rect_intersected(s->get_rect(), span->get_rect());
                if(!ints1 && !ints2)
                    ++ i;
                else if(ints1 && ints2)
                    spans.insert(std::next(i), s);
                else if(ints2)
                    i = spans.insert(++ i, s);
            }
        }
    }
    rtr.insert(span, span->get_rect());
}

void loop_blinn_processor::split_quadratic(lb_line* line1, lb_line* line2, lb_joint* sp[5])
{
    assert(line1 && line2 && sp);
    auto* j1 = line1->get_prev_joint();
    auto* j2 = line2->get_prev_joint();
    auto* j3 = line2->get_next_joint();
    assert(j1 && j2 && j3);
    vec2 o[3], p[5];
    o[0] = j1->get_point();
    o[1] = j2->get_point();
    o[2] = j3->get_point();
    pink::split_quad_bezier(p, o, 0.5f);
    auto* nj1 = create_joint<lb_control_joint>(p[3]);
    auto* nj2 = create_joint<lb_end_joint>(p[4]);
    auto* nl1 = create_line();
    auto* nl2 = create_line();
    auto* nextline = line2->get_next_line();
    lb_connect(j3, nl1, nj1);
    lb_connect(nj1, nl2, nj2);
    lb_connect(nj2, nextline);
    j2->set_point(p[1]);
    j3->set_point(p[2]);
    sp[0] = j1;
    sp[1] = j2;
    sp[2] = j3;
    sp[3] = nj1;
    sp[4] = nj2;
}

void loop_blinn_processor::split_cubic(lb_line* line1, lb_line* line2, lb_line* line3, lb_joint* sp[7], float t)
{
    assert(line1 && line2 && line3 && sp);
    auto* j1 = line1->get_prev_joint();
    auto* j2 = line2->get_prev_joint();
    auto* j3 = line3->get_prev_joint();
    auto* j4 = line3->get_next_joint();
    assert(j1 && j2 && j3 && j4);
    vec2 o[4], p[7];
    o[0] = j1->get_point();
    o[1] = j2->get_point();
    o[2] = j3->get_point();
    o[3] = j4->get_point();
    pink::split_cubic_bezier(p, o, t);
    auto* nj1 = create_joint<lb_control_joint>(p[4]);
    auto* nj2 = create_joint<lb_control_joint>(p[5]);
    auto* nj3 = create_joint<lb_end_joint>(p[6]);
    auto* nl1 = create_line();
    auto* nl2 = create_line();
    auto* nl3 = create_line();
    auto* nextline = line3->get_next_line();
    lb_connect(j4, nl1, nj1);
    lb_connect(nj1, nl2, nj2);
    lb_connect(nj2, nl3, nj3);
    lb_connect(nj3, nextline);
    j2->set_point(p[1]);
    j3->set_point(p[2]);
    j4->set_point(p[3]);
    sp[0] = j1;
    sp[1] = j2;
    sp[2] = j3;
    sp[3] = j4;
    sp[4] = nj1;
    sp[5] = nj2;
    sp[6] = nj3;
}

int loop_blinn_processor::try_split_cubic(lb_line* line1, lb_line* line2, lb_line* line3, lb_joint* sp[7], float t)
{
    assert(line1 && line2 && line3 && sp);
    assert(t > 0.f && t < 1.f);
    auto* j1 = line1->get_prev_joint();
    auto* j2 = line2->get_prev_joint();
    auto* j3 = line3->get_prev_joint();
    auto* j4 = line3->get_next_joint();
    assert(j1 && j2 && j3 && j4);
    vec2 o[4], p[7];
    o[0] = j1->get_point();
    o[1] = j2->get_point();
    o[2] = j3->get_point();
    o[3] = j4->get_point();
    pink::split_cubic_bezier(p, o, t);
    auto* nextline = line3->get_next_line();
    auto* prevjoint = j1->get_prev_joint();
    auto* nextjoint = nextline->get_next_joint();
    assert(nextline && prevjoint && nextjoint);
    if(prevjoint->get_type() != lbt_end_joint)
        prevjoint = 0;
    if(nextjoint->get_type() != lbt_end_joint)
        nextjoint = 0;
    auto is_straight1 = [](const vec2& p1, const vec2& p2, const vec2& p3)->bool {
        float d = pink::point_line_distance(p2, p1, p3);
        return abs(d) < 0.1f;
    };
    auto is_straight2 = [](const vec2& p1, const vec2& p2, const vec2& p3, const vec2& p4)->bool {
        float d1 = pink::point_line_distance(p2, p1, p3);
        float d2 = pink::point_line_distance(p3, p2, p4);
        return abs(d1) < 0.1f && abs(d2) < 0.1f;
    };
    static const bool enable_merge_segs = false;
    if(is_straight2(p[0], p[1], p[2], p[3])) {
        if(enable_merge_segs && prevjoint && is_straight1(prevjoint->get_point(), p[0], p[3])) {
            j1->set_point(p[3]);
            sp[0] = j1;
            if(is_straight2(p[3], p[4], p[5], p[6])) {
                if(enable_merge_segs && nextjoint && is_straight1(p[3], p[6], nextjoint->get_point())) {
                    lb_connect(j1, nextline);
                    return 1;
                }
                else {
                    lb_connect(j1, line1, j4);
                    assert((j4->get_point() == p[6]) && (j4->get_next_line() == nextline));
                    sp[1] = j4;
                    return 2;
                }
            }
            else {
                j2->set_point(p[4]);
                j3->set_point(p[5]);
                assert((j4->get_point() == p[6]) && (j4->get_next_line() == nextline));
                sp[1] = j2;
                sp[2] = j3;
                sp[3] = j4;
                return 4;
            }
        }
        else {
            j4->set_point(p[3]);
            lb_connect(j1, line1, j4);
            sp[0] = j1;
            sp[1] = j4;
            if(is_straight2(p[3], p[4], p[5], p[6])) {
                if(enable_merge_segs && nextjoint && is_straight1(p[3], p[6], nextjoint->get_point())) {
                    assert(j4->get_next_line() == nextline);
                    return 2;
                }
                else {
                    auto* nj1 = create_joint<lb_end_joint>(p[6]);
                    lb_connect(j4, line2, nj1);
                    lb_connect(nj1, nextline);
                    sp[2] = nj1;
                    return 3;
                }
            }
            else {
                auto* nj1 = create_joint<lb_end_joint>(p[6]);
                auto* nl1 = create_line();
                j2->set_point(p[4]);
                j3->set_point(p[5]);
                lb_connect(j4, line2, j2);
                lb_connect(j2, line3, j3);
                lb_connect(j3, nl1, nj1);
                lb_connect(nj1, nextline);
                sp[2] = j2;
                sp[3] = j3;
                sp[4] = nj1;
                return 5;
            }
        }
    }
    else {
        j2->set_point(p[1]);
        j3->set_point(p[2]);
        j4->set_point(p[3]);
        sp[0] = j1;
        sp[1] = j2;
        sp[2] = j3;
        sp[3] = j4;
        if(is_straight2(p[3], p[4], p[5], p[6])) {
            if(enable_merge_segs && nextjoint && is_straight1(p[3], p[6], nextjoint->get_point())) {
                assert(j4->get_next_line() == nextline);
                return 4;
            }
            else {
                auto* nj1 = create_joint<lb_end_joint>(p[6]);
                auto* nl1 = create_line();
                lb_connect(j4, nl1, nj1);
                sp[4] = nj1;
                return 5;
            }
        }
        else {
            auto* nj1 = create_joint<lb_control_joint>(p[4]);
            auto* nj2 = create_joint<lb_control_joint>(p[5]);
            auto* nj3 = create_joint<lb_end_joint>(p[6]);
            auto* nl1 = create_line();
            auto* nl2 = create_line();
            auto* nl3 = create_line();
            lb_connect(j4, nl1, nj1);
            lb_connect(nj1, nl2, nj2);
            lb_connect(nj2, nl3, nj3);
            lb_connect(nj3, nextline);
            sp[4] = nj1;
            sp[5] = nj2;
            sp[6] = nj3;
            return 7;
        }
    }
}

lb_span* loop_blinn_processor::split_rtree_span(lb_span* span)
{
    assert(span);
    auto t = span->get_type();
    if(t == lst_quad) {
        lb_joint* sp[5];
        auto* p = static_cast<lb_quad_span*>(span);
        split_quadratic(p->get_line(0), p->get_line(1), sp);
        p->setup(sp[0]->get_next_line(), sp[1]->get_next_line());
        auto* s = gs_new(lb_quad_span, sp[2]->get_next_line(), sp[3]->get_next_line());
        _span_holdings.push_back(s);
        return s;
    }
    else if(t == lst_cubic) {
        lb_joint* sp[7];
        auto* p = static_cast<lb_cubic_span*>(span);
        split_cubic(p->get_line(0), p->get_line(1), p->get_line(2), sp, 0.5f);
        p->setup(sp[0]->get_next_line(), sp[1]->get_next_line(), sp[2]->get_next_line());
        auto* s = gs_new(lb_cubic_span, sp[3]->get_next_line(), sp[4]->get_next_line(), sp[5]->get_next_line());
        _span_holdings.push_back(s);
        return s;
    }
    assert(!"unexpected.");
    return 0;
}

void loop_blinn_processor::split_span_recursively(lb_polygon* poly, lb_span* span)
{
    assert(poly && span);
    auto* s = split_rtree_span(span);
    assert(s);
    check_rtree_span(poly, span);
    check_rtree_span(poly, s);
}

void loop_blinn_processor::calc_klm_coords()
{
    mat3 m(
        2.f / _width, 0.f, 0.f,
        0.f, -2.f / _height, 0.f,
        -1.f, 1.f, 1.f
        );
    for(auto* p : _polygons) {
        assert(p);
        p->convert_to_ncoord(m);
        calc_klm_coords(p);
    }
}

void loop_blinn_processor::calc_klm_coords(lb_polygon* poly)
{
    assert(poly);
    auto* boundary = poly->get_boundary();
    assert(boundary);
    calc_klm_coords(poly, boundary);
    auto& holes = poly->get_holes();
    for(auto* p : holes)
        calc_klm_coords(poly, p);
}

void loop_blinn_processor::calc_klm_coords(lb_polygon* poly, lb_line* start)
{
    assert(poly && start);
    auto* line = calc_klm_span(poly, start);
    while(line != start)
        line = calc_klm_span(poly, line);
}

lb_line* loop_blinn_processor::calc_klm_span(lb_polygon* poly, lb_line* line)
{
    assert(poly && line);
    lb_line_list span;
    auto* next = lb_get_span(span, line);
    int size = (int)span.size();
    if(size == 1)
        return next;
    if(size == 2) {
        auto* line1 = span.at(0);
        auto* line2 = span.at(1);
        auto* j1 = line1->get_prev_joint();
        auto* j2 = line2->get_prev_joint();
        auto* j3 = line2->get_next_joint();
        assert(j1 && j2 && j3);
        assert(j1->get_type() == lbt_end_joint);
        assert(j2->get_type() == lbt_control_joint);
        assert(j3->get_type() == lbt_end_joint);
        auto* ej1 = static_cast<lb_end_joint*>(j1);
        auto* cj2 = static_cast<lb_control_joint*>(j2);
        auto* ej3 = static_cast<lb_end_joint*>(j3);
        ej1->set_klm(1, vec3(0.f, 0.f, 1.f));
        cj2->set_klm(vec3(0.5f, 0.f, 1.f));
        ej3->set_klm(0, vec3(1.f, 1.f, 1.f));
    }
    else {
        assert(size == 3);
        auto* line1 = span.at(0);
        auto* line2 = span.at(1);
        auto* line3 = span.at(2);
        auto* j1 = line1->get_prev_joint();
        auto* j2 = line2->get_prev_joint();
        auto* j3 = line3->get_prev_joint();
        auto* j4 = line3->get_next_joint();
        assert(j1 && j2 && j3 && j4);
        assert(j1->get_type() == lbt_end_joint);
        assert(j2->get_type() == lbt_control_joint);
        assert(j3->get_type() == lbt_control_joint);
        assert(j4->get_type() == lbt_end_joint);
        vec3 m[4];
        float sp = pink::get_cubic_klmcoords(m, j1->get_ncoord_point(), j2->get_ncoord_point(), j3->get_ncoord_point(), j4->get_ncoord_point(),
            j1->get_point(), j2->get_point(), j3->get_point(), j4->get_point()
            );
        if(sp < 0.f) {
            auto* ej1 = static_cast<lb_end_joint*>(j1);
            auto* cj2 = static_cast<lb_control_joint*>(j2);
            auto* cj3 = static_cast<lb_control_joint*>(j3);
            auto* ej4 = static_cast<lb_end_joint*>(j4);
            ej1->set_klm(1, m[0]);
            cj2->set_klm(m[1]);
            cj3->set_klm(m[2]);
            ej4->set_klm(0, m[3]);
        }
        else {
            lb_joint* spj[7];
            int c = try_split_cubic(line1, line2, line3, spj, sp);
            mat3 m(
                2.f / _width, 0.f, -1.f,
                0.f, -2.f / _height, 1.f,
                0.f, 0.f, 1.f
                );
            if(c == 1) {
                auto& p = spj[0]->get_point();
                spj[0]->set_ncoord_point(vec2().transformcoord(p, m));
                return spj[0]->get_next_line();     /* the line was casted. */
            }
            for(int i = 0; i < c; i ++) {
                auto& p = spj[i]->get_point();
                spj[i]->set_ncoord_point(vec2().transformcoord(p, m));
            }
            assert(line == line1);
            return line;
        }
    }
    return next;
}

__ariel_end__
