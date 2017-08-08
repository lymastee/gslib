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
#include <pink/clip.h>

__pink_begin__

static void trace_patch_or_polygon(clip_result::iterator i)
{
    assert(i);
    auto& p = *i;
    if(p.is_patch()) {
        static_cast<clip_patch&>(p).tracing();
        return;
    }
    p.tracing();
}

clip_joint* clip_joint::get_prev_joint() const
{
    assert(_prev);
    return _prev->get_joint(0);
}

clip_joint* clip_joint::get_next_joint() const
{
    assert(_next);
    return _next->get_joint(1);
}

clip_line* clip_joint::get_prev_above(float y) const
{
    auto* prev = get_prev_joint();
    assert(prev);
    if(prev->get_point().y > y)
        return get_prev_line();
    return 0;
}

clip_line* clip_joint::get_next_above(float y) const
{
    auto* next = get_next_joint();
    assert(next);
    if(next->get_point().y > y)
        return get_next_line();
    return 0;
}

clip_line* clip_joint::get_prev_below(float y) const
{
    auto* prev = get_prev_joint();
    assert(prev);
    if(prev->get_point().y < y)
        return get_prev_line();
    return 0;
}

clip_line* clip_joint::get_next_below(float y) const
{
    auto* next = get_next_joint();
    assert(next);
    if(next->get_point().y < y)
        return get_next_line();
    return 0;
}

clip_end_joint::clip_end_joint(const vec2& pt)
{
    _point = pt;
    _info[0] = _info[1] = 0;
}

void clip_end_joint::set_path_info(int i, path_info* pnf)
{
    assert(pnf);
    assert(i == 0 || i == 1);
    _info[i] = pnf;
}

clip_interpolate_joint::clip_interpolate_joint(const vec2& pt)
{
    _point = pt;
    _ratio = 0.f;
    _info = 0;
}

clip_intersect_joint::clip_intersect_joint(const vec2& pt)
{
    _point = pt;
    _orient[0] = _orient[1] = 0;
    _cut[0] = _cut[1] = 0;
    _symmetric = 0;
}

path_info* clip_intersect_joint::get_path_info() const
{
    auto* prevline = get_prev_line();
    assert(prevline);
    auto* prevj = prevline->get_another_joint(this);
    assert(prevj);
    auto t = prevj->get_type();
    switch(t)
    {
    case ct_interpolate_joint:
        {
            auto* joint = static_cast<clip_interpolate_joint*>(prevj);
            return joint->get_path_info();
        }
    case ct_end_joint:
        {
            auto* joint = static_cast<clip_end_joint*>(prevj);
            return joint->get_path_info(1);
        }
    default:
        assert(!"unexpected in clip_intersect_joint::get_path_info.");
        return 0;
    }
}

clip_joint* clip_line::get_another_joint(const clip_joint* joint) const
{
    if(_joint[0] == joint)
        return _joint[1];
    else if(_joint[1] == joint)
        return _joint[0];
    assert(!"unexpected.");
    return 0;
}

clip_line* clip_line::get_prev_line() const
{
    if(!_joint[0])
        return 0;
    assert(_joint[0]->get_next_line() == this);
    return _joint[0]->get_prev_line();
}

clip_line* clip_line::get_next_line() const
{
    if(!_joint[1])
        return 0;
    assert(_joint[1]->get_prev_line() == this);
    return _joint[1]->get_next_line();
}

void clip_line::tracing() const
{
    const vec2& p1 = get_point(0);
    const vec2& p2 = get_point(1);
    trace(_t("@moveTo %f, %f;"), p1.x, p1.y);
    trace(_t("@lineTo %f, %f;\n"), p2.x, p2.y);
}

template<class _node, class _list>
static void destroy_ptr_list(_list& l)
{
    if(l.empty())
        return;
    std::for_each(l.begin(), l.end(), [](_list::value_type& ptr) {
        assert(ptr);
        gs_del(_node, ptr);
    });
    l.clear();
}

clip_polygon::~clip_polygon()
{
    destroy_ptr_list<clip_joint, clip_joints>(_joint_holdings);
    destroy_ptr_list<clip_line, clip_lines>(_line_holdings);
    destroy_ptr_list<path_info, clip_path_infos>(_pnf_holdings);
}

int clip_polygon::create(const painter_path& path, int start)
{
    int cap = path.size();
    if(start >= cap)
        return cap;
    int next = create_path_infos(path, start);
    if(!_pnf_holdings.empty())
        create_polygon();
    return next;
}

void clip_polygon::convert_from(clip_patch& patch)
{
    _joint_holdings.swap(patch._joint_holdings);
    _line_holdings.swap(patch._line_holdings);
    _pnf_holdings.swap(patch._pnf_holdings);
    set_line_start(patch.get_line_start());
}

void clip_polygon::reverse_direction()
{
    assert(_line_start);
    auto reverse_segment = [](clip_line* line)->clip_line* {
        assert(line);
        auto* joint = line->get_joint(1);
        assert(joint);
        auto* next = joint->get_next_line();
        joint->reverse();
        line->reverse();
        return next;
    };
    auto* line = reverse_segment(_line_start);
    while(line != _line_start)
        line = reverse_segment(line);
    /* make sure that the first joint was not a control point */
    auto* joint1 = _line_start->get_joint(0);
    assert(joint1);
    auto t = joint1->get_type();
    if(t == ct_final_joint) {
        auto* fj1 = static_cast<clip_final_joint*>(joint1);
        if(fj1->is_control_point()) {
            auto* joint2 = _line_start->get_joint(1);
            assert(joint2 && joint2->get_type() == ct_final_joint);
            auto* fj2 = static_cast<clip_final_joint*>(joint2);
            assert(!fj2->is_control_point());
            _line_start = _line_start->get_next_line();
        }
    }
}

void clip_polygon::tracing() const
{
    if(!_line_start)
        return;
    trace(_t("@!\n"));
    const clip_line* first = _line_start;
    assert(first);
    const vec2& p1 = first->get_point(0);
    const vec2& p2 = first->get_point(1);
    trace(_t("@moveTo %f, %f;\n"), p1.x, p1.y);
    trace(_t("@lineTo %f, %f;\n"), p2.x, p2.y);
    for(auto* line = first->get_next_line(); line != first; line = line->get_next_line()) {
        assert(line);
        const vec2& p = line->get_point(1);
        trace(_t("@lineTo %f, %f;\n"), p.x, p.y);
    }
    trace(_t("@@\n"));
}

void clip_polygon::trace_segments() const
{
    if(!_line_start)
        return;
    trace(_t("@!\n"));
    const clip_line* first = _line_start;
    assert(first);
    const vec2& p1 = first->get_point(0);
    const vec2& p2 = first->get_point(1);
    trace(_t("@moveTo %f, %f;\n"), p1.x, p1.y);
    trace(_t("@lineTo %f, %f;\n"), p2.x, p2.y);
    for(auto* line = first->get_next_line(); line != first; line = line->get_next_line()) {
        const vec2& p1 = line->get_point(0);
        const vec2& p2 = line->get_point(1);
        trace(_t("@moveTo %f, %f;\n"), p1.x, p1.y);
        trace(_t("@lineTo %f, %f;\n"), p2.x, p2.y);
    }
    trace(_t("@@\n"));
}

void clip_polygon::trace_final() const
{
    assert(_line_start);
    trace(_t("@!\n"));
    auto trace_once = [](clip_line* line)->clip_line* {
        assert(line);
        auto* joint1 = line->get_joint(0);
        auto* joint2 = line->get_joint(1);
        assert(joint1 && joint2);
        assert(joint1->get_type() == ct_final_joint && joint2->get_type() == ct_final_joint);
        auto* fj1 = static_cast<clip_final_joint*>(joint1);
        auto* fj2 = static_cast<clip_final_joint*>(joint2);
        assert(!fj1->is_control_point());
        auto& p1 = fj1->get_point();
        auto& p2 = fj2->get_point();
        // trace(_t("@moveTo %f, %f;\n"), p1.x, p1.y);
        if(!fj2->is_control_point()) {
            trace(_t("@lineTo %f, %f;\n"), p2.x, p2.y);
            return line->get_next_line();
        }
        auto* line2 = line->get_next_line();
        auto* joint3 = line2->get_joint(1);
        assert(joint3 && joint3->get_type() == ct_final_joint);
        auto* fj3 = static_cast<clip_final_joint*>(joint3);
        auto& p3 = fj3->get_point();
        if(!fj3->is_control_point()) {
            trace(_t("@quadTo %f, %f, %f, %f;\n"), p2.x, p2.y, p3.x, p3.y);
            return line2->get_next_line();
        }
        auto* line3 = line2->get_next_line();
        auto* joint4 = line3->get_joint(1);
        assert(joint4 && joint4->get_type() == ct_final_joint);
        auto* fj4 = static_cast<clip_final_joint*>(joint4);
        auto& p4 = fj4->get_point();
        assert(!fj4->is_control_point());
        trace(_t("@cubicTo %f, %f, %f, %f, %f, %f;\n"), p2.x, p2.y, p3.x, p3.y, p4.x, p4.y);
        return line3->get_next_line();
    };
    auto* joint1 = _line_start->get_joint(0);
    assert(joint1);
    auto& p = joint1->get_point();
    trace(_t("@moveTo %f, %f;\n"), p.x, p.y);
    auto* line = trace_once(_line_start);
    for(; line != _line_start; line = trace_once(line));
    trace(_t("@@\n"));
}

int clip_polygon::create_path_infos(const painter_path& path, int start)
{
    int cap = path.size();
    if(start >= cap)
        return cap;
    const painter_node* first = path.get_node(start);
    assert(first && first->get_tag() == painter_path::pt_moveto);
    const painter_node* last = first;
    int i = start + 1;
    for(; i < cap; i ++) {
        const painter_node* node = path.get_node(i);
        assert(node);
        auto tag = node->get_tag();
        if(tag == painter_path::pt_moveto)
            break;
        _pnf_holdings.push_back(gs_new(path_info, last, node));
        last = node;
    }
    assert(first != last && first->get_point() == last->get_point());
    return i;
}

void clip_polygon::create_polygon()
{
    assert(!_pnf_holdings.empty());
    int size = (int)_pnf_holdings.size();
    if(size == 1) {
        path_info* pnf = _pnf_holdings.front();
        assert(pnf && pnf->get_order() > 1 && "must be curve.");
        clip_end_joint* joint = create_end_joint(pnf, pnf);
        assert(joint);
        create_segment(joint, pnf, pnf, joint);
    }
    else if(size == 2) {
        path_info* pnf1 = _pnf_holdings.front();
        path_info* pnf2 = _pnf_holdings.back();
        assert(pnf1 && pnf2);
        clip_end_joint* joint1 = create_end_joint(pnf2, pnf1);
        clip_end_joint* joint2 = create_end_joint(pnf1, pnf2);
        assert(joint1 && joint2);
        create_segment(joint1, pnf1, pnf2, joint2);
        create_segment(joint2, pnf2, pnf1, joint1);
    }
    else {
        auto i = _pnf_holdings.begin(), back = --_pnf_holdings.end();
        path_info* pnf1 = *i, *pnf2 = *back;
        assert(pnf1 && pnf2);
        clip_end_joint* joint1 = create_end_joint(pnf2, pnf1);
        clip_end_joint* lastj = joint1;
        while(i != back) {
            auto j = std::next(i);
            path_info* pnf = *i;
            path_info* nextpnf = *j;
            lastj = create_segment(lastj, pnf, nextpnf);
            i = j;
        }
        create_segment(lastj, pnf2, pnf1, joint1);
    }
    if(!_line_holdings.empty())
        _line_start = _line_holdings.front();
}

clip_end_joint* clip_polygon::create_end_joint(path_info* pnf1, path_info* pnf2)
{
    assert(pnf1 && pnf2);
    vec2 pt[4];
    int c = pnf1->get_point_count();
    assert(c <= 4);
    pnf1->get_points(pt, c);
    clip_end_joint* j = gs_new(clip_end_joint, pt[c - 1]);
    assert(j);
    j->set_path_info(0, pnf1);
    j->set_path_info(1, pnf2);
    _joint_holdings.push_back(j);
    return j;
}

clip_interpolate_joint* clip_polygon::create_interpolate_joint(path_info* pnf, const vec2& p, float t)
{
    assert(pnf);
    clip_interpolate_joint* j = gs_new(clip_interpolate_joint, p);
    assert(j);
    j->set_path_info(pnf);
    j->set_ratio(t);
    _joint_holdings.push_back(j);
    return j;
}

clip_line* clip_polygon::create_line(clip_joint* joint1, clip_joint* joint2)
{
    clip_line* e = gs_new(clip_line);
    assert(e);
    e->set_joint(0, joint1);
    e->set_joint(1, joint2);
    if(joint1) joint1->set_next_line(e);
    if(joint2) joint2->set_prev_line(e);
    _line_holdings.push_back(e);
    return e;
}

clip_end_joint* clip_polygon::create_segment(clip_joint* joint1, path_info* pnf1, path_info* pnf2, clip_joint* joint2)
{
    assert(joint1 && pnf1 && pnf2);
    /* create interpolate segments */
    clip_joint* lastj = joint1;
    int order = pnf1->get_order();
    if(order != 1) {
        vec2 pt[4];
        int c = pnf1->get_point_count();
        assert(c == 3 || c == 4);
        pnf1->get_points(pt, c);
        if(c == 3) {
            vec3 para[2];
            get_quad_parameter_equation(para, pt[0], pt[1], pt[2]);
            int step = get_rough_interpolate_step(pt[0], pt[1], pt[2]);
            float t, chord;
            t = chord = 1.f / (step - 1);
            for(int i = 1; i < step - 1; i ++, t += chord) {
                vec2 p;
                eval_quad(p, para, t);
                clip_joint* joint = create_interpolate_joint(pnf1, p, t);
                assert(joint);
                create_line(lastj, joint);
                lastj = joint;
            }
        }
        else {
            assert(c == 4);
            vec4 para[2];
            get_cubic_parameter_equation(para, pt[0], pt[1], pt[2], pt[3]);
            int step = get_rough_interpolate_step(pt[0], pt[1], pt[2], pt[3]);
            float t, chord;
            t = chord = 1.f / (step - 1);
            for(int i = 1; i < step - 1; i ++, t += chord) {
                vec2 p;
                eval_cubic(p, para, t);
                clip_joint* joint = create_interpolate_joint(pnf1, p, t);
                assert(joint);
                create_line(lastj, joint);
                lastj = joint;
            }
        }
    }
    /* join end joint */
    if(!joint2)
        joint2 = create_end_joint(pnf1, pnf2);
    assert(joint2);
    create_line(lastj, joint2);
    return static_cast<clip_end_joint*>(joint2);
}

clip_mirror_joint* clip_polygon::create_mirror_joint(clip_joint* joint)
{
    assert(joint);
    clip_mirror_joint* p = gs_new(clip_mirror_joint);
    assert(p);
    p->set_mirror(joint);
    _joint_holdings.push_back(p);
    return p;
}

clip_final_joint* clip_polygon::create_final_joint(clip_joint* joint)
{
    /* joint could be null, which indicates a control point. */
    clip_final_joint* p = gs_new(clip_final_joint);
    assert(p);
    p->set_mirror(joint);
    _joint_holdings.push_back(p);
    return p;
}

void clip_polygon::adopt(clip_polygon& poly)
{
    _joint_holdings.insert(_joint_holdings.end(), poly._joint_holdings.begin(), poly._joint_holdings.end());
    _line_holdings.insert(_line_holdings.end(), poly._line_holdings.begin(), poly._line_holdings.end());
    _pnf_holdings.insert(_pnf_holdings.end(), poly._pnf_holdings.begin(), poly._pnf_holdings.end());
    poly._joint_holdings.clear();
    poly._line_holdings.clear();
    poly._pnf_holdings.clear();
}

clip_sweep_line::~clip_sweep_line()
{
    for(auto* p : _joint_holdings) { gs_del(clip_sweep_joint, p); }
    _sorted_by_x.clear();
    _joint_holdings.clear();
}

void clip_sweep_line::add_joint(clip_sweep_joint* joint)
{
    assert(joint);
    _joint_holdings.push_back(joint);
    _sorted_by_x.insert(joint);
}

clip_sweep_joint* clip_sweep_line::create_joint(clip_joint* joint)
{
    assert(joint);
    auto* p = gs_new(clip_sweep_endpoint, joint);
    add_joint(p);
    return p;
}

static bool clip_range_test_y(clip_line* line, float y)
{
    assert(line);
    auto* p = line->get_joint(0);
    auto* q = line->get_joint(1);
    assert(p && q);
    float m = p->get_point().y;
    float n = q->get_point().y;
    if(m < n)
        return y > m && y < n;
    return y > n && y < m;
}

static float calc_sweep_line_x(clip_joint* upside, clip_joint* downside, float y)
{
    assert(upside && downside);
    const vec2& p1 = upside->get_point();
    const vec2& p2 = downside->get_point();
    return p2.x - ((p2.y - y) / (p2.y - p1.y)) * (p2.x - p1.x);
}

clip_sweep_joint* clip_sweep_line::create_joint(clip_line* line, float y)
{
    assert(line);
    if(!clip_range_test_y(line, y))
        return 0;
    float x = calc_sweep_line_x(line->get_joint(0), line->get_joint(1), y);
    auto* p = gs_new(clip_sweep_relay);
    p->set_line(line);
    p->set_point(vec2(x, y));
    add_joint(p);
    return p;
}

clip_sweep_joint* clip_sweep_line::ensure_unique_create(clip_joint* joint)
{
    assert(joint);
    assert(joint->get_type() == ct_end_joint);
    const vec2& p = joint->get_point();
    auto f = _sorted_by_x.find(&clip_sweep_key(p));
    auto end = _sorted_by_x.end();
    if(f == end)
        return create_joint(joint);
    bool found = false;
    for(; f != end; ++ f) {
        auto* j = *f;
        if(j->get_point().x != p.x)
            break;
        if(j->get_tag() == cst_endpoint) {
            auto* k = static_cast<clip_sweep_endpoint*>(j);
            if(k->get_joint() == joint) {
                found = true;
                break;
            }
        }
    }
    return found ? 0 : create_joint(joint);
}

const vec2& clip_sweeper::get_another_point() const
{
    auto tag = joint->get_tag();
    if(tag == cst_endpoint) {
        auto* p = static_cast<clip_sweep_endpoint*>(joint);
        auto* j = p->get_joint();
        auto* r = line->get_another_joint(j);
        assert(r);
        return r->get_point();
    }
    else if(tag == cst_relay) {
        assert(line);
        auto* p = line->get_joint(0);
        return p->get_point();
    }
    else {
        assert(!"unexpected.");
        static const vec2 p;
        return p;
    }
}

void clip_sweeper::tracing() const
{
    assert(line);
    line->tracing();
    assert(joint);
    const vec2& p = joint->get_point();
    trace(_t("@dot %f, %f;\n"), p.x, p.y);
}

clip_mirror_joint* clip_patch::create_assumed_mj(const clip_sweeper& sweeper)
{
    if(sweeper.joint->get_tag() == cst_endpoint) {
        auto* j = static_cast<clip_sweep_endpoint*>(sweeper.joint)->get_joint();
        assert(j);
        return create_mirror_joint(j);
    }
    return 0;
}

clip_line* clip_patch::fix_line_front(clip_line* line, clip_mirror_joint* mj)
{
    assert(line && line->get_joint(1));
    if(auto* j = line->get_joint(0))
        return create_line(mj, j);
    if(mj) {
        line->set_joint(0, mj);
        mj->set_next_line(line);
    }
    return line;
}

clip_line* clip_patch::fix_line_back(clip_line* line, clip_mirror_joint* mj)
{
    assert(line && line->get_joint(0));
    if(auto* j = line->get_joint(1))
        return create_line(j, mj);
    if(mj) {
        line->set_joint(1, mj);
        mj->set_prev_line(line);
    }
    return line;
}

void clip_patch::reverse_direction()
{
    auto* line1 = _line_start;
    auto* line2 = _line_end;
    auto sweeper1 = _start_point;
    auto sweeper2 = _end_point;
    auto* line = line1;
    if(auto* joint = line->get_joint(0))
        joint->reverse();
    for(;;) {
        auto* next = line->get_next_line();
        auto* joint = line->get_joint(1);
        line->reverse();
        if(joint)
            joint->reverse();
        if(line == line2)
            break;
        line = next;
    }
    set_line_start(line2);
    set_line_end(line1);
    set_start_point(sweeper2);
    set_end_point(sweeper1);
}

void clip_patch::finish_patch()
{
    auto* line1 = _line_start;
    auto* line2 = _line_end;
    assert(line1 && line2);
    auto* j0 = line1->get_joint(0);
    auto* j1 = line2->get_joint(1);
    if(!j0 || !j1)
        return;     // todo: why?
    assert(j0 && j1);
    if(j0 != j1) {
        assert(!j0->get_prev_line() && !j1->get_next_line());
        create_line(j1, j0);
    }
}

void clip_patch::tracing() const
{
    if(!_line_start)
        return;
    trace(_t("@!\n"));
    auto* first = _line_start;
    assert(first);
    auto* firstj = first->get_joint(0);
    if(firstj) {
        const vec2& p1 = firstj->get_point();
        trace(_t("@moveTo %f, %f;\n"), p1.x, p1.y);
    }
    else {
        const vec2& p1 = first->get_point(1);
        trace(_t("@moveTo %f, %f;\n"), p1.x, p1.y);
        first = first->get_next_line();
    }
    for(auto* line = first; line && line != _line_end; line = line->get_next_line()) {
        const vec2& p = line->get_point(1);
        trace(_t("@lineTo %f, %f;\n"), p.x, p.y);
    }
    auto* joint = _line_end->get_joint(1);
    if(joint) {
        const vec2& p = joint->get_point();
        trace(_t("@lineTo %f, %f;\n"), p.x, p.y);
    }
    auto& p1 = _start_point.get_point();
    auto& p2 = _end_point.get_point();
    trace(_t("@cross %f, %f;\n"), p1.x, p1.y);
    trace(_t("@cross %f, %f;\n"), p2.x, p2.y);
    trace(_t("@@\n"));
}

clip_sweep_line_algorithm::clip_sweep_line_algorithm()
{
    _clip_tag = cp_exclude;
}

void clip_sweep_line_algorithm::destroy()
{
    for(auto* p : _sweep_lines) { gs_del(clip_sweep_line, p); }
    _sweep_lines.clear();
    destroy_ptr_list<clip_joint, clip_joints>(_joint_holdings);
    destroy_ptr_list<clip_line, clip_lines>(_line_holdings);
    _sorted_by_y.clear();
    _intersections.clear();
}

void clip_sweep_line_algorithm::add_polygon(clip_polygon* polygon)
{
    assert(polygon);
    auto& joints = polygon->get_joints();
    for(auto* p : joints)
        _sorted_by_y.insert(p);
}

void clip_sweep_line_algorithm::proceed()
{
    prepare_sweep_lines();
    proceed_overlapped_intersections();
    proceed_intersections();
    finish_intersections();
}

static bool is_clockwise(const clip_polygon* poly)
{
    assert(poly);
    float x = -FLT_MAX;
    clip_joint* detectp = 0;
    auto* line1 = poly->get_line_start();
    assert(line1);
    auto* joint1 = line1->get_joint(0);
    assert(joint1);
    auto& p1 = joint1->get_point();
    if(p1.x > x) {
        x = p1.x;
        detectp = joint1;
    }
    for(auto* line = line1->get_next_line(); line != line1; line = line->get_next_line()) {
        auto* joint = line->get_joint(0);
        assert(joint);
        auto& p = joint->get_point();
        if(p.x > x) {
            x = p.x;
            detectp = joint;
        }
    }
    assert(detectp);
    auto& d1 = detectp->get_prev_joint()->get_point();
    auto& d2 = detectp->get_point();
    auto& d3 = detectp->get_next_joint()->get_point();
    return !is_concave_angle(d1, d2, d3);
}

void fix_clockwise(clip_result::iterator p);
void fix_count_clockwise(clip_result::iterator p);

static void fix_clockwise(clip_result::iterator p)
{
    assert(p);
    auto* poly = &*p;
    assert(poly);
    if(!is_clockwise(poly))
        poly->reverse_direction();
    for(auto i = p.child(); i; i.to_next())
        fix_count_clockwise(i);
}

static void fix_count_clockwise(clip_result::iterator p)
{
    assert(p);
    auto* poly = &*p;
    assert(poly);
    if(is_clockwise(poly))
        poly->reverse_direction();
    for(auto i = p.child(); i; i.to_next())
        fix_clockwise(i);
}

static void fix_clockwise_logics(clip_result& result)
{
    auto r = result.get_root();
    assert(r);
    for(auto i = r.child(); i; i.to_next())
        fix_clockwise(i);
}

void clip_sweep_line_algorithm::output(clip_result& result)
{
    switch(get_clip_tag())
    {
    case cp_union:
        output_union(result);
        break;
    case cp_intersect:
        output_intersect(result);
        break;
    case cp_exclude:
        output_exclude(result);
        break;
    default:
        assert(!"output failed: unexpected.");
        return;
    }

//     trace(_t("@@@@@@@@@@@@@@@@@@@@@@@!\n"));
//     result.preorder_traversal([](clip_result_wrapper* w) {
//         assert(w);
//         if(!w->parent())
//             return;
//         w->get_ptr()->tracing();
//     });
    assert(_spliters.empty());
    create_spliters();
    replace_curves(result);
    fix_clockwise_logics(result);
}

void clip_sweep_line_algorithm::prepare_sweep_lines()
{
    if(_sorted_by_y.empty())
        return;
    assert(_sorted_by_y.size() >= 2);
    auto i = _sorted_by_y.begin();
    auto* last = create_sweep_line(*i);
    for(++ i; i != _sorted_by_y.end(); ++ i) {
        auto y = (*i)->get_point().y;
        if(abs(y - last->get_y()) < 1e-5f)
            last->ensure_unique_create(*i);
        else {
            auto* curr = create_sweep_line(*i);
            proceed_sweep_line(last, curr);
            last = curr;
        }
    }
}

void clip_sweep_line_algorithm::proceed_overlapped_intersections()
{
    if(_sweep_lines.empty())
        return;
    for(auto* p : _sweep_lines)
        proceed_overlapped_intersection(p);
}

void clip_sweep_line_algorithm::proceed_intersections()
{
    if(_sweep_lines.empty())
        return;
    assert(_sweep_lines.size() >= 2);
    auto i = _sweep_lines.begin();
    auto j = std::next(i);
    for(; j != _sweep_lines.end(); i = j ++)
        proceed_intersection(i, j);
}

void clip_sweep_line_algorithm::finish_intersections()
{
    for(auto* p : _joint_holdings) {
        assert(p && (p->get_type() == ct_intersect_joint));
        finish_intersection(static_cast<clip_intersect_joint*>(p));
    }
}

void clip_sweep_line_algorithm::trace_sweep_lines() const
{
    if(_sweep_lines.empty())
        return;
    trace(_t("@!\n"));
    for(auto* p : _sweep_lines)
        trace_sweep_line(p);
    trace(_t("@@\n"));
}

void clip_sweep_line_algorithm::trace_sweep_line(const clip_sweep_line* line) const
{
    assert(line);
    auto& joints = line->const_sorted_joints();
    if(joints.empty())
        return;
    for(auto i = joints.begin();;) {
        assert(i != joints.end());
        auto j = std::next(i);
        if(j == joints.end())
            break;
        auto& p = (*i)->get_point();
        auto& q = (*j)->get_point();
        trace(_t("@moveTo %f, %f;\n"), p.x, p.y);
        trace(_t("@lineTo %f, %f;\n"), q.x, q.y);
        i = j;
    }
}

void clip_sweep_line_algorithm::trace_sweep_lines(const mat3& m) const
{
    if(_sweep_lines.empty())
        return;
    trace(_t("@!\n"));
    for(auto* p : _sweep_lines)
        trace_sweep_line(p, m);
    trace(_t("@@\n"));
}

void clip_sweep_line_algorithm::trace_sweep_line(const clip_sweep_line* line, const mat3& m) const
{
    assert(line);
    auto& joints = line->const_sorted_joints();
    if(joints.empty())
        return;
    painter_path path;
    for(auto i = joints.begin();;) {
        assert(i != joints.end());
        auto j = std::next(i);
        if(j == joints.end())
            break;
        auto& p = (*i)->get_point();
        auto& q = (*j)->get_point();
        path.move_to(p);
        path.line_to(q);
        i = j;
    }
    path.transform(m);
    path.tracing();
}

void clip_sweep_line_algorithm::trace_intersections() const
{
    for(auto* p : _intersections) {
        const vec2& pt = p->get_point();
        trace(_t("@dot %f, %f;\n"), pt.x, pt.y);
    }
}

clip_sweep_line* clip_sweep_line_algorithm::create_sweep_line(clip_joint* joint)
{
    assert(joint);
    assert(joint->get_prev_line() && joint->get_next_line());
    auto* prev = joint->get_prev_line();
    auto* next = joint->get_next_line();
    assert(prev && next);
    auto* line = gs_new(clip_sweep_line, joint->get_point().y);
    line->create_joint(joint);
    _sweep_lines.push_back(line);
    return line;
}

void clip_sweep_line_algorithm::proceed_sweep_line(clip_sweep_line* last, clip_sweep_line* curr)
{
    assert(last && curr);
    auto& csj = curr->const_sorted_joints();
    assert(csj.size() == 1);
    auto* fst = *csj.begin();
    assert(fst && (fst->get_tag() == cst_endpoint));
    auto* fstep = static_cast<clip_sweep_endpoint*>(fst);
    auto* fstc = fstep->get_joint();
    assert(fstc);
    auto& sorted_joints = last->get_sorted_joints();
    for(auto* p : sorted_joints) {
        assert(p);
        auto tag = p->get_tag();
        if(tag == cst_endpoint) {
            auto* ptr = static_cast<clip_sweep_endpoint*>(p);
            auto* joint = ptr->get_joint();
            assert(joint);
            float y = curr->get_y();
            if(auto* prev = joint->get_prev_above(y))
                curr->create_joint(prev, y);
            if(auto* next = joint->get_next_above(y))
                curr->create_joint(next, y);
        }
        else if(tag == cst_relay) {
            auto* ptr = static_cast<clip_sweep_relay*>(p);
            auto* line = ptr->get_line();
            auto* a = line->get_joint(0);
            auto* b = line->get_joint(1);
            auto& p1 = a->get_point();
            auto& p2 = b->get_point();
            float y = curr->get_y();
            if(abs(p1.y - y) < 1e-5f) {
                if(fstc != a)
                    curr->create_joint(a);
            }
            else if(abs(p2.y - y) < 1e-5f) {
                if(fstc != b)
                    curr->create_joint(b);
            }
            else
                curr->create_joint(line, y);
        }
        else {
            assert(!"unexpected.");
        }
    }
}

void clip_sweep_line_algorithm::proceed_sweep_line(clip_sweep_line* line, clip_sweepers::iterator from, clip_sweepers::iterator to)
{
    assert(line);
    float y = line->get_y();
    for(auto i = from; i != to; ++ i)
        line->create_joint(i->line, y);
}

void clip_sweep_line_algorithm::proceed_overlapped_intersection(clip_sweep_line* line)
{
    assert(line);
    // todo:
    // no x-coincide points in sweep lines
    // replace them with clip_sl_pivot
}

static bool is_upside_corner(clip_sweep_joint* joint)
{
    assert(joint);
    auto tag = joint->get_tag();
    if(tag == cst_relay)
        return false;
    else if(tag == cst_endpoint) {
        auto* p = static_cast<clip_sweep_endpoint*>(joint);
        auto* q = p->get_joint();
        assert(q);
        float y = q->get_point().y;
        auto* i = q->get_prev_joint();
        auto* j = q->get_next_joint();
        assert(i && j);
        return (i->get_point().y < y) && (j->get_point().y < y);
    }
    else {
        assert(!"unexpected.");
        return false;
    }
}

static bool is_downside_corner(clip_sweep_joint* joint)
{
    assert(joint);
    auto tag = joint->get_tag();
    if(tag == cst_relay)
        return false;
    else if(tag == cst_endpoint) {
        auto* p = static_cast<clip_sweep_endpoint*>(joint);
        auto* q = p->get_joint();
        assert(q);
        float y = q->get_point().y;
        auto* i = q->get_prev_joint();
        auto* j = q->get_next_joint();
        assert(i && j);
        return (i->get_point().y > y) && (j->get_point().y > y);
    }
    else {
        assert(!"unexpected.");
        return false;
    }
}

static void create_upside_endpoint(clip_sweepers& sweepers, float y0, float y1, clip_sweep_endpoint* joint, clip_joint* raw)
{
    assert(joint && raw);
    auto* m = raw->get_prev_joint();
    auto* n = raw->get_next_joint();
    assert(m && n);
    if(m->get_point().y > y0) {
        if(n->get_point().y > y0) {
            float yy = (y0 + y1) * 0.5f;
            float x1 = calc_sweep_line_x(raw, m, yy);
            float x2 = calc_sweep_line_x(raw, n, yy);
            if(x1 > x2) {
                sweepers.push_back(clip_sweeper(joint, raw->get_next_line()));
                sweepers.push_back(clip_sweeper(joint, raw->get_prev_line()));
            }
            else {
                sweepers.push_back(clip_sweeper(joint, raw->get_prev_line()));
                sweepers.push_back(clip_sweeper(joint, raw->get_next_line()));
            }
        }
        else {
            sweepers.push_back(clip_sweeper(joint, raw->get_prev_line()));
        }
    }
    else if(n->get_point().y > y0)
        sweepers.push_back(clip_sweeper(joint, raw->get_next_line()));
}

static void create_upside_endpoint(clip_sweepers& sweepers, float y0, float y1, clip_sweep_endpoint* joint, clip_intersect_joint* raw)
{
    assert(joint && raw);
    auto* symm = raw->get_symmetric();
    assert(symm);
    auto* m1 = raw->get_prev_joint();
    auto* m2 = raw->get_next_joint();
    auto* n1 = symm->get_prev_joint();
    auto* n2 = symm->get_next_joint();
    assert(m1 && m2 && n1 && n2);
    clip_sweeper swp1, swp2;
    if(m1->get_point().y > y0) {
        assert(m2->get_point().y < y0);
        swp1.joint = joint;
        swp1.line = raw->get_prev_line();
    }
    else {
        assert(m2->get_point().y > y0);
        swp1.joint = joint;
        swp1.line = raw->get_next_line();
    }
    if(n1->get_point().y > y0) {
        assert(n2->get_point().y < y0);
        swp2.joint = joint;
        swp2.line = symm->get_prev_line();
    }
    else {
        assert(n2->get_point().y > y0);
        swp2.joint = joint;
        swp2.line = symm->get_next_line();
    }
    float y = (y0 + y1) * 0.5f;
    float x1 = calc_sweep_line_x(m1, m2, y);
    float x2 = calc_sweep_line_x(n1, n2, y);
    if(x1 < x2) {
        sweepers.push_back(swp1);
        sweepers.push_back(swp2);
    }
    else {
        sweepers.push_back(swp2);
        sweepers.push_back(swp1);
    }
}

static void create_upside_endpoint(clip_sweepers& sweepers, float y0, float y1, clip_sweep_endpoint* joint)
{
    assert(joint);
    auto* p = joint->get_joint();
    auto tag = p->get_type();
    switch(tag)
    {
    case ct_end_joint:
    case ct_interpolate_joint:
        create_upside_endpoint(sweepers, y0, y1, joint, p);
        break;
    case ct_intersect_joint:
        create_upside_endpoint(sweepers, y0, y1, joint, static_cast<clip_intersect_joint*>(p));
        break;
    default:
        assert(!"todo pivot.");
    }
}

static void create_upside_relay(clip_sweepers& sweepers, clip_sweep_relay* joint)
{
    assert(joint);
    sweepers.push_back(clip_sweeper(joint, joint->get_line()));
}

static void create_upside_sweeper(clip_sweepers& sweepers, float y0, float y1, clip_sweep_joint* joint)
{
    assert(joint);
    if(is_upside_corner(joint))
        return;
    auto tag = joint->get_tag();
    switch(tag)
    {
    case cst_endpoint:
        create_upside_endpoint(sweepers, y0, y1, static_cast<clip_sweep_endpoint*>(joint));
        break;
    case cst_relay:
        create_upside_relay(sweepers, static_cast<clip_sweep_relay*>(joint));
        break;
    default:
        assert(!"unexpected.");
    }
}

static void create_upside_sweepers(clip_sweepers& sweepers, clip_sweep_line* line, clip_sweep_line* other)
{
    assert(line && other);
    auto& joints = line->get_sorted_joints();
    float y0 = line->get_y();
    float y1 = other->get_y();
    std::for_each(joints.begin(), joints.end(), [&](clip_sweep_joint* joint) {
        create_upside_sweeper(sweepers, y0, y1, joint);
    });
}

static void create_downside_endpoint(clip_sweepers& sweepers, float y0, float y1, clip_sweep_endpoint* joint, clip_joint* raw)
{
    assert(joint && raw);
    auto* m = raw->get_prev_joint();
    auto* n = raw->get_next_joint();
    assert(m && n);
    if(m->get_point().y < y0) {
        if(n->get_point().y < y0) {
            float yy = (y0 + y1) * 0.5f;
            float x1 = calc_sweep_line_x(raw, m, yy);
            float x2 = calc_sweep_line_x(raw, n, yy);
            if(x1 > x2) {
                sweepers.push_back(clip_sweeper(joint, raw->get_next_line()));
                sweepers.push_back(clip_sweeper(joint, raw->get_prev_line()));
            }
            else {
                sweepers.push_back(clip_sweeper(joint, raw->get_prev_line()));
                sweepers.push_back(clip_sweeper(joint, raw->get_next_line()));
            }
        }
        else {
            sweepers.push_back(clip_sweeper(joint, raw->get_prev_line()));
        }
    }
    else if(n->get_point().y < y0)
        sweepers.push_back(clip_sweeper(joint, raw->get_next_line()));
}

static void create_downside_endpoint(clip_sweepers& sweepers, float y0, float y1, clip_sweep_endpoint* joint, clip_intersect_joint* raw)
{
    assert(joint && raw);
    auto* symm = raw->get_symmetric();
    assert(symm);
    auto* m1 = raw->get_prev_joint();
    auto* m2 = raw->get_next_joint();
    auto* n1 = symm->get_prev_joint();
    auto* n2 = symm->get_next_joint();
    assert(m1 && m2 && n1 && n2);
    clip_sweeper swp1, swp2;
    if(m1->get_point().y < y0) {
        assert(m2->get_point().y > y0);
        swp1.joint = joint;
        swp1.line = raw->get_prev_line();
    }
    else {
        assert(m2->get_point().y < y0);
        swp1.joint = joint;
        swp1.line = raw->get_next_line();
    }
    if(n1->get_point().y < y0) {
        assert(n2->get_point().y > y0);
        swp2.joint = joint;
        swp2.line = symm->get_prev_line();
    }
    else {
        assert(n2->get_point().y < y0);
        swp2.joint = joint;
        swp2.line = symm->get_next_line();
    }
    float y = (y0 + y1) * 0.5f;
    float x1 = calc_sweep_line_x(m1, m2, y);
    float x2 = calc_sweep_line_x(n1, n2, y);
    if(x1 < x2) {
        sweepers.push_back(swp1);
        sweepers.push_back(swp2);
    }
    else {
        sweepers.push_back(swp2);
        sweepers.push_back(swp1);
    }
}

static void create_downside_endpoint(clip_sweepers& sweepers, float y0, float y1, clip_sweep_endpoint* joint)
{
    assert(joint);
    auto* p = joint->get_joint();
    auto tag = p->get_type();
    switch(tag)
    {
    case ct_end_joint:
    case ct_interpolate_joint:
        create_downside_endpoint(sweepers, y0, y1, joint, p);
        break;
    case ct_intersect_joint:
        create_downside_endpoint(sweepers, y0, y1, joint, static_cast<clip_intersect_joint*>(p));
        break;
    default:
        assert(!"todo pivot");
    }
}

static void create_downside_relay(clip_sweepers& sweepers, clip_sweep_relay* joint)
{
    assert(joint);
    sweepers.push_back(clip_sweeper(joint, joint->get_line()));
}

static void create_downside_sweeper(clip_sweepers& sweepers, float y0, float y1, clip_sweep_joint* joint)
{
    assert(joint);
    if(is_downside_corner(joint))
        return;
    auto tag = joint->get_tag();
    switch(tag)
    {
    case cst_endpoint:
        create_downside_endpoint(sweepers, y0, y1, static_cast<clip_sweep_endpoint*>(joint));
        break;
    case cst_relay:
        create_downside_relay(sweepers, static_cast<clip_sweep_relay*>(joint));
        break;
    default:
        assert(!"unexpected.");
    }
}

static void create_downside_sweepers(clip_sweepers& sweepers, clip_sweep_line* line, clip_sweep_line* other)
{
    assert(line && other);
    auto& joints = line->get_sorted_joints();
    float y0 = line->get_y();
    float y1 = other->get_y();
    std::for_each(joints.begin(), joints.end(), [&](clip_sweep_joint* joint) {
        create_downside_sweeper(sweepers, y0, y1, joint);
    });
}

static void trace_sweepers(const clip_sweepers& sweepers)
{
    trace(_t("@!\n"));
    trace(_t("#tracing sweepers.\n"));
    for(auto& p : sweepers)
        p.tracing();
    trace(_t("@@\n"));
}

static void trace_sweepers(clip_sweep_line* line1, clip_sweep_line* line2)
{
    assert(line1 && line2);
    clip_sweepers up, down;
    create_upside_sweepers(up, line1, line2);
    create_downside_sweepers(down, line2, line1);
    trace_sweepers(up);
    trace_sweepers(down);
}

static void trace_sweepers(clip_sweep_lines& sweeplines)
{
    if(sweeplines.empty())
        return;
    auto last = sweeplines.begin();
    auto i = std::next(last);
    for(; i != sweeplines.end(); last = i ++)
        trace_sweepers(*last, *i);
}

void clip_sweep_line_algorithm::proceed_intersection(iterator a, iterator b)
{
    assert(b == std::next(a));
    assert(b != _sweep_lines.end());
    clip_sweepers up, down;
    create_upside_sweepers(up, *a, *b);
    create_downside_sweepers(down, *b, *a);
    auto i = up.begin();
    auto j = down.begin();
    for(; i != up.end() && j != down.end(); ++ i, ++ j) {
        if(i->line != j->line) {
            auto line = insert_sweep_line(a, b);
            proceed_intersection(*line, *i, *j, a, b);
            proceed_sweep_line(*line, up.begin(), i);
            proceed_sweep_line(*line, std::next(i), up.end());
            proceed_overlapped_intersection(*line);
            proceed_intersection(a, line);
            proceed_intersection(line, b);
            break;
        }
    }
}

class clip_front_tracer
{
public:
    typedef clip_sweep_lines::iterator iterator;
    clip_front_tracer() {}
    clip_front_tracer(iterator f, iterator t): _from(f), _to(t) {}
    clip_sweep_line* get_line() const { return *_from; }
    template<class _lamb>
    void run(_lamb lamb) const
    {
        for(auto i = _from;; -- i) {
            if(!lamb(*i) || i == _to)
                break;
        }
    }

protected:
    iterator        _from, _to;
};

class clip_back_tracer
{
public:
    typedef clip_sweep_lines::iterator iterator;
    clip_back_tracer() {}
    clip_back_tracer(iterator f, iterator t): _from(f), _to(t) {}
    clip_sweep_line* get_line() const { return _from == _to ? 0 : *_from; }
    template<class _lamb>
    void run(_lamb lamb) const
    {
        for(auto i = _from; i != _to; ++ i) {
            if(!lamb(*i))
                break;
        }
    }

protected:
    iterator        _from, _to;
};

static void clip_connect(clip_joint* joint, clip_line* line)
{
    assert(joint && line);
    joint->set_next_line(line);
    line->set_joint(0, joint);
}

static void clip_connect(clip_line* line, clip_joint* joint)
{
    assert(joint && line);
    joint->set_prev_line(line);
    line->set_joint(1, joint);
}

static void clip_connect(clip_joint* joint1, clip_line* line, clip_joint* joint2)
{
    assert(joint1 && line && joint2);
    joint1->set_next_line(line);
    joint2->set_prev_line(line);
    line->set_joint(0, joint1);
    line->set_joint(1, joint2);
}

static void clip_connect(clip_line* line1, clip_joint* joint, clip_line* line2)
{
    assert(line1 && joint && line2);
    joint->set_prev_line(line1);
    joint->set_next_line(line2);
    line1->set_joint(1, joint);
    line2->set_joint(0, joint);
}

static bool clip_replace_line(clip_sweep_line* line, clip_line* from, clip_line* to)
{
    assert(line && from && to);
    float x = calc_sweep_line_x(from->get_joint(0), from->get_joint(1), line->get_y());
    auto& m = line->get_sorted_joints();
    auto lb = m.lower_bound(&clip_sweep_key(vec2(x - 1e-3f, 0.f)));
    auto ub = m.upper_bound(&clip_sweep_key(vec2(x + 1e-3f, 0.f)));
    if(ub != m.end())
        ++ ub;
    for(auto i = lb; i != ub; ++ i) {
        auto tag = (*i)->get_tag();
        if(tag == cst_relay) {
            auto* ptr = static_cast<clip_sweep_relay*>(*i);
            if(ptr->get_line() == from) {
                ptr->set_line(to);
                return true;
            }
        }
    }
    return false;
}

static void clip_insert(clip_line* org, clip_joint* insj, clip_line* insl)
{
    assert(org && insj && insl);
    auto* orgj1 = org->get_joint(0);
    auto* orgj2 = org->get_joint(1);
    assert(orgj1 && orgj2);
    clip_connect(orgj1, org, insj);
    clip_connect(insj, insl, orgj2);
}

static void clip_insert(clip_sweeper& sweeper, clip_joint* insj, clip_line* insl, clip_front_tracer& ftracer, clip_back_tracer& btracer)
{
    assert(insj && insl);
    clip_line* replace_from = sweeper.line;
    clip_line* replace_to = insl;
    clip_sweep_line* line1 = ftracer.get_line();
    clip_sweep_line* line2 = btracer.get_line();
    auto tag = sweeper.joint->get_tag();
    auto* line = sweeper.line;
    const vec2& p1 = line->get_point(0);
    const vec2& p2 = line->get_point(1);
    assert((fuzzy_less_inclusive(p1.y, line1->get_y(), 1e-5f) && fuzzy_greater_inclusive(p2.y, line2->get_y(), 1e-5f)) ||
        (fuzzy_less_inclusive(p2.y, line1->get_y(), 1e-5f) && fuzzy_greater_inclusive(p1.y, line2->get_y(), 1e-5f))
        );
    bool replace_upward = p1.y > p2.y;
    clip_insert(sweeper.line, insj, insl);
    if(replace_upward) {
        ftracer.run([&replace_from, &replace_to](clip_sweep_line* line)->bool {
            return clip_replace_line(line, replace_from, replace_to);
        });
    }
    else {
        btracer.run([&replace_from, &replace_to](clip_sweep_line* line)->bool {
            return clip_replace_line(line, replace_from, replace_to);
        });
    }
}

static void clip_setup_intersect_info(clip_intersect_joint* joint, clip_intersect_joint* symm, clip_line* line_j, clip_line* line_s)
{
    assert(joint && symm && line_j && line_s);
    auto* j1 = line_j->get_joint(0);
    auto* j2 = line_j->get_joint(1);
    auto* s1 = line_s->get_joint(0);
    auto* s2 = line_s->get_joint(1);
    assert(j1 && j2 && s1 && s2);
    joint->set_orient(0, j1);
    joint->set_orient(1, j2);
    joint->set_cut(0, s1);
    joint->set_cut(1, s2);
    symm->set_orient(0, s1);
    symm->set_orient(1, s2);
    symm->set_cut(0, j1);
    symm->set_cut(1, j2);
}

void clip_sweep_line_algorithm::proceed_intersection(clip_sweep_line* line, clip_sweeper& sweeper1, clip_sweeper& sweeper2, iterator a, iterator b)
{
    assert(line);
    assert((a != _sweep_lines.end()) && (b != _sweep_lines.end()));
    vec2 p[2], q[2];
    p[0] = sweeper1.get_point();
    p[1] = sweeper1.get_another_point();
    q[0] = sweeper2.get_point();
    q[1] = sweeper2.get_another_point();
    vec2 d[2];
    d[0].sub(p[1], p[0]);
    d[1].sub(q[1], q[0]);
    vec2 ip;
    intersectp_linear_linear(ip, p[0], q[0], d[0], d[1]);
    line->set_y(ip.y);
    auto* joint = gs_new(clip_intersect_joint, ip);
    auto* symm = gs_new(clip_intersect_joint, ip);
    assert(joint && symm);
    _joint_holdings.push_back(joint);
    _joint_holdings.push_back(symm);
    joint->set_symmetric(symm);
    symm->set_symmetric(joint);
    _sorted_by_y.insert(joint);
    _intersections.push_back(joint);
    line->create_joint(joint);
    auto* hand1 = gs_new(clip_line);
    auto* hand2 = gs_new(clip_line);
    assert(hand1 && hand2);
    _line_holdings.push_back(hand1);
    _line_holdings.push_back(hand2);
    clip_setup_intersect_info(joint, symm, sweeper1.line, sweeper2.line);
    clip_insert(sweeper1, joint, hand1, clip_front_tracer(a, _sweep_lines.begin()), clip_back_tracer(b, _sweep_lines.end()));
    clip_insert(sweeper2, symm, hand2, clip_front_tracer(a, _sweep_lines.begin()), clip_back_tracer(b, _sweep_lines.end()));
}

clip_sweep_line_algorithm::iterator clip_sweep_line_algorithm::insert_sweep_line(iterator i, iterator j)
{
    clip_sweep_line* line = gs_new(clip_sweep_line);
    assert(line);
    return _sweep_lines.insert(j, line);
}

static clip_joint* clip_find_prev(clip_joint* joint)
{
    assert(joint);
    auto tag = joint->get_type();
    if((tag != ct_intersect_joint) && (tag != ct_pivot_joint))
        return joint;
    for(auto* j = joint->get_prev_joint(); j && j != joint; j = j->get_prev_joint()) {
        auto tag = j->get_type();
        if((tag != ct_intersect_joint) && (tag != ct_pivot_joint))
            return j;
    }
    return 0;
}

static clip_joint* clip_find_next(clip_joint* joint)
{
    assert(joint);
    auto tag = joint->get_type();
    if((tag != ct_intersect_joint) && (tag != ct_pivot_joint))
        return joint;
    for(auto* j = joint->get_next_joint(); j && j != joint; j = j->get_next_joint()) {
        auto tag = j->get_type();
        if((tag != ct_intersect_joint) && (tag != ct_pivot_joint))
            return j;
    }
    return 0;
}

void clip_sweep_line_algorithm::finish_intersection(clip_intersect_joint* joint)
{
    assert(joint);
    auto* j1 = clip_find_prev(joint->get_orient(0));
    auto* j2 = clip_find_next(joint->get_orient(1));
    assert(j1 && j2);
    joint->set_orient(0, j1);
    joint->set_orient(1, j2);
    auto* s1 = clip_find_prev(joint->get_cut(0));
    auto* s2 = clip_find_next(joint->get_cut(1));
    assert(s1 && s2);
    joint->set_cut(0, s1);
    joint->set_cut(1, s2);
    auto* symm = joint->get_symmetric();
    assert(symm);
    symm->set_orient(0, s1);
    symm->set_orient(1, s2);
    symm->set_cut(0, j1);
    symm->set_cut(1, j2);
}

static bool is_neighbour_joint(clip_joint* i, clip_joint* j)
{
    assert(i && j);
    if(i == j)
        return true;
    if((i->get_type() == ct_intersect_joint) && (j->get_type() == ct_intersect_joint)) {
        auto* p1 = static_cast<clip_intersect_joint*>(i);
        auto* p2 = static_cast<clip_intersect_joint*>(j);
        if(p1->get_symmetric() == p2)
            return true;
    }
    auto* l1 = i->get_next_line();
    auto* l2 = j->get_prev_line();
    auto* l3 = i->get_prev_line();
    auto* l4 = j->get_next_line();
    return l1 == l2 || l3 == l4 || l1 == l3 || l2 == l4;
}

static bool is_neighbour_joint(clip_sweep_joint* i, clip_sweep_joint* j)
{
    assert(i && j);
    if((i->get_tag() == cst_relay) || (j->get_tag() == cst_relay))
        return false;
    auto* p1 = static_cast<clip_sweep_endpoint*>(i);
    auto* p2 = static_cast<clip_sweep_endpoint*>(j);
    assert(p1 && p2);
    return is_neighbour_joint(p1->get_joint(), p2->get_joint());
}

void clip_sweep_line_algorithm::output_exclude(clip_result& result)
{
    assert(!_sweep_lines.empty());
    clip_assembler_exclude assembler(result);
    assembler.proceed(_sweep_lines);
}

void clip_sweep_line_algorithm::output_union(clip_result& result)
{
}

void clip_sweep_line_algorithm::output_intersect(clip_result& result)
{
}

void clip_sweep_line_algorithm::destroy_spliters()
{
    for(auto& p : _spliters)
        delete p.second;
    _spliters.clear();
}

void clip_sweep_line_algorithm::destroy_fixed_points()
{
    if(!_fixed_points.empty())
        _fixed_points.clear();
}

static path_info* retrieve_path_info(clip_joint* joint1, clip_joint* joint2)
{
    assert(joint1 && joint2);
    auto t1 = joint1->get_type();
    auto t2 = joint2->get_type();
    if(t1 == ct_interpolate_joint) {
        auto* p = static_cast<clip_interpolate_joint*>(joint1);
        return p->get_path_info();
    }
    else if(t2 == ct_interpolate_joint) {
        auto* p = static_cast<clip_interpolate_joint*>(joint2);
        return p->get_path_info();
    }
    else if(t1 == ct_end_joint && t2 == ct_end_joint) {
        auto* p1 = static_cast<clip_end_joint*>(joint1);
        auto* p2 = static_cast<clip_end_joint*>(joint2);
        if((p1->get_path_info(1) == p2->get_path_info(0)) || (p1->get_path_info(1) == p2->get_path_info(1)))
            return p1->get_path_info(1);
        else if((p1->get_path_info(0) == p2->get_path_info(1)) || (p1->get_path_info(0) == p2->get_path_info(0)))
            return p1->get_path_info(0);
    }
    assert(!"unexpected situation.");
    return 0;
}

void clip_sweep_line_algorithm::create_spliters()
{
    assert(_spliters.empty());
    for(auto* p : _intersections) {
        assert(p->get_type() == ct_intersect_joint);
        auto* ptr = static_cast<clip_intersect_joint*>(p);
        auto* pnf1 = retrieve_path_info(ptr->get_orient(0), ptr->get_orient(1));
        auto* pnf2 = retrieve_path_info(ptr->get_cut(0), ptr->get_cut(1));
        create_spliter(ptr, pnf1, pnf2);
    }
}

static int select_map_point(vec2 pt[], int c, clip_intersect_joint* p)
{
    assert(pt && p);
    auto& chk = p->get_point();
    float d = FLT_MAX;
    int r = -1;
    for(int i = 0; i < c; i ++) {
        vec2 v;
        v.sub(pt[i], chk);
        float d1 = sqrtf(v.x * v.x + v.y * v.y);
        if(d1 < d)
            d = d1, r = i;
    }
    return r;
}

static curve_spliter* query_or_create_spliter(curve_spliter_map& csm, path_info* pnf)
{
    assert(pnf);
    auto*& spliter = csm[pnf];
    if(!spliter)
        spliter = curve_helper::create_spliter(pnf);
    return spliter;
}

static void create_spliter_curve_straight(curve_spliter_map& csm, clip_fixed_points& fps, clip_intersect_joint* p, path_info* pnf1, path_info* pnf2)
{
    assert(p && pnf1 && pnf2);
    assert(p->get_path_info() == pnf1);
    auto* s = p->get_symmetric();
    assert(s);
    assert(pnf1->get_order() > 1 && pnf2->get_order() == 1);
    vec2 line[2];
    vec2 curve[4];
    pnf1->get_points(curve, _countof(curve));
    pnf2->get_points(line, _countof(line));
    vec3 linear;
    vec2 d;
    get_linear_coefficient(linear, line[0], d.sub(line[1], line[0]));
    int o = pnf1->get_order();
    float t[3];
    vec2 pt[3];
    int sel = -1;
    if(o == 2) {
        vec3 para[2];
        get_quad_parameter_equation(para, curve[0], curve[1], curve[2]);
        int c = intersection_quad_linear(t, para, linear);
        for(int i = 0; i < c; i ++)
            eval_quad(pt[i], para, t[i]);
        sel = select_map_point(pt, c, p);
    }
    else if(o == 3) {
        vec4 para[2];
        get_cubic_parameter_equation(para, curve[0], curve[1], curve[2], curve[3]);
        int c = intersection_cubic_linear(t, para, linear);
        for(int i = 0; i < c; i ++)
            eval_cubic(pt[i], para, t[i]);
        sel = select_map_point(pt, c, p);
    }
    else {
        assert(!"unexpected curve order.");
        return;
    }
    assert(sel >= 0);
    auto* spliter1 = query_or_create_spliter(csm, pnf1);
    assert(spliter1);
    curve_helper::create_next_spliter(vec2(), spliter1, t[sel]);
    fps.insert(std::make_pair(static_cast<clip_joint*>(p), pt[sel]));
    fps.insert(std::make_pair(static_cast<clip_joint*>(p->get_symmetric()), pt[sel]));
}

static void create_spliter_quad_quad(curve_spliter_map& csm, clip_fixed_points& fps, clip_intersect_joint* p, path_info* pnf1, path_info* pnf2)
{
    assert(p && pnf1 && pnf2);
    assert(pnf1->get_order() == 2 && pnf2->get_order() == 2);
    vec3 para1[2], para2[2];
    vec2 quad1[3], quad2[3];
    float ts[4][2];
    vec2 pt[4];
    pnf1->get_points(quad1, _countof(quad1));
    pnf2->get_points(quad2, _countof(quad2));
    get_quad_parameter_equation(para1, quad1[0], quad1[1], quad1[2]);
    get_quad_parameter_equation(para2, quad2[0], quad2[1], quad2[2]);
    int c = intersection_quad_quad(ts, para1, para2);
    for(int i = 0; i < c; i ++)
        eval_quad(pt[i], para1, ts[i][0]);
    int sel = select_map_point(pt, c, p);
    assert(sel >= 0);
    auto* spliter1 = query_or_create_spliter(csm, pnf1);
    auto* spliter2 = query_or_create_spliter(csm, pnf2);
    assert(spliter1 && spliter2);
    curve_helper::create_next_spliter(vec2(), spliter1, ts[sel][0]);
    curve_helper::create_next_spliter(vec2(), spliter2, ts[sel][1]);
    fps.insert(std::make_pair(static_cast<clip_joint*>(p), pt[sel]));
    fps.insert(std::make_pair(static_cast<clip_joint*>(p->get_symmetric()), pt[sel]));
}

static void create_spliter_cubic_cubic(curve_spliter_map& csm, clip_fixed_points& fps, clip_intersect_joint* p, path_info* pnf1, path_info* pnf2)
{
    assert(p && pnf1 && pnf2);
    assert(pnf1->get_order() == 3 && pnf2->get_order() == 3);
    vec2 cubic1[4], cubic2[4];
    vec2 pt[9];
    pnf1->get_points(cubic1, _countof(cubic1));
    pnf2->get_points(cubic2, _countof(cubic2));
    int c = intersectp_cubic_cubic(pt, cubic1, cubic2, 0.2f);
    int sel = select_map_point(pt, c, p);
    assert(sel >= 0);
    auto* spliter1 = query_or_create_spliter(csm, pnf1);
    auto* spliter2 = query_or_create_spliter(csm, pnf2);
    assert(spliter1 && spliter2);
    float t = spliter1->reparameterize(pt[sel]);
    float s = spliter2->reparameterize(pt[sel]);
    curve_helper::create_next_spliter(vec2(), spliter1, t);
    curve_helper::create_next_spliter(vec2(), spliter2, s);
    fps.insert(std::make_pair(static_cast<clip_joint*>(p), pt[sel]));
    fps.insert(std::make_pair(static_cast<clip_joint*>(p->get_symmetric()), pt[sel]));
}

static void create_spliter_quad_cubic(curve_spliter_map& csm, clip_fixed_points& fps, clip_intersect_joint* p, path_info* pnf1, path_info* pnf2)
{
    assert(p && pnf1 && pnf2);
    assert(pnf1->get_order() == 2 && pnf2->get_order() == 3);
    vec2 quad[3], cubic[4];
    vec2 pt[6];
    pnf1->get_points(quad, _countof(quad));
    pnf2->get_points(cubic, _countof(cubic));
    int c = intersectp_cubic_quad(pt, cubic, quad, 0.2f);
    int sel = select_map_point(pt, c, p);
    assert(sel >= 0);
    auto* spliter1 = query_or_create_spliter(csm, pnf1);
    auto* spliter2 = query_or_create_spliter(csm, pnf2);
    assert(spliter1 && spliter2);
    float t = spliter1->reparameterize(pt[sel]);
    float s = spliter2->reparameterize(pt[sel]);
    curve_helper::create_next_spliter(vec2(), spliter1, t);
    curve_helper::create_next_spliter(vec2(), spliter2, s);
    fps.insert(std::make_pair(static_cast<clip_joint*>(p), pt[sel]));
    fps.insert(std::make_pair(static_cast<clip_joint*>(p->get_symmetric()), pt[sel]));
}

static void create_spliter_curve_curve(curve_spliter_map& csm, clip_fixed_points& fps, clip_intersect_joint* p, path_info* pnf1, path_info* pnf2)
{
    assert(p && pnf1 && pnf2);
    assert(p->get_path_info() == pnf1);
    auto* s = p->get_symmetric();
    assert(s);
    int o1 = pnf1->get_order();
    int o2 = pnf2->get_order();
    assert(o1 > 1 && o2 > 1);
    if(o1 == 2) {
        return o2 == 2 ? create_spliter_quad_quad(csm, fps, p, pnf1, pnf2) :
            create_spliter_quad_cubic(csm, fps, p, pnf1, pnf2);
    }
    else {
        assert(o1 == 3);
        return o2 == 2 ? create_spliter_quad_cubic(csm, fps, s, pnf2, pnf1) :
            create_spliter_cubic_cubic(csm, fps, p, pnf1, pnf2);
    }
}

void clip_sweep_line_algorithm::create_spliter(clip_intersect_joint* p, path_info* pnf1, path_info* pnf2)
{
    assert(p && pnf1 && pnf2);
    int o1 = pnf1->get_order();
    int o2 = pnf2->get_order();
    bool p1_is_curve = (o1 > 1);
    bool p2_is_curve = (o2 > 1);
    if(!p1_is_curve && !p2_is_curve)
        return;
    else if(p1_is_curve && !p2_is_curve)
        return create_spliter_curve_straight(_spliters, _fixed_points, p, pnf1, pnf2);
    else if(!p1_is_curve && p2_is_curve)
        return create_spliter_curve_straight(_spliters, _fixed_points, p->get_symmetric(), pnf2, pnf1);
    return create_spliter_curve_curve(_spliters, _fixed_points, p, pnf1, pnf2);
}

void clip_sweep_line_algorithm::replace_curves(clip_result& result)
{
    auto r = result.get_root();
    assert(r);
    if(r.is_leaf()) /* empty path */
        return;
    for(auto i = r.child();; i.to_next()) {
        replace_curves(result, i);
        if(i == r.last_child())
            break;
    }
}

void clip_sweep_line_algorithm::replace_curves(clip_result& result, clip_result::iterator p)
{
    assert(p);
    replace_curves(*p);
    if(p.is_leaf())
        return;
    for(auto i = p.child();; i.to_next()) {
        replace_curves(result, i);
        if(i == p.last_child())
            break;
    }
}

template<class _lambda>
static bool is_mirror_specific_joint(clip_joint* joint, _lambda lamb)
{
    assert(joint);
    auto t = joint->get_type();
    if(t != ct_mirror_joint)
        return false;
    auto* mj = static_cast<clip_mirror_joint*>(joint);
    auto* m = mj->get_mirror();
    assert(m);
    return lamb(m);
}

template<class _lambda>
static clip_joint* find_next_mirror_specific_joint(clip_joint* joint, clip_joint* endofsearch, _lambda lamb)
{
    assert(joint && endofsearch);
    for(; joint != endofsearch; joint = joint->get_next_joint()) {
        if(is_mirror_specific_joint(joint, lamb))
            return joint;
    }
    return 0;
}

void clip_sweep_line_algorithm::replace_curves(clip_polygon& poly)
{
    auto* linestart = poly.get_line_start();
    assert(linestart);
    auto* startj = linestart->get_joint(0);
    assert(startj);
    auto is_end_or_intersected = [](clip_joint* p)-> bool {
        assert(p);
        auto t = p->get_type();
        return t == ct_end_joint || t == ct_intersect_joint;
    };
    if(!is_mirror_specific_joint(startj, is_end_or_intersected)) {
        auto* f = find_next_mirror_specific_joint(linestart->get_joint(1), startj, is_end_or_intersected);
        if(!f)  /* why? */
            return;
        startj = f;
    }
    auto* nextj = find_next_mirror_specific_joint(startj->get_next_joint(), startj, is_end_or_intersected);
    if(!nextj) {
        replace_curve(poly, startj, startj);
        return;
    }
    replace_curve(poly, startj, nextj);
    auto* joint = nextj;
    for(;;) {
        nextj = find_next_mirror_specific_joint(joint->get_next_joint(), startj, is_end_or_intersected);
        if(!nextj)
            break;
        replace_curve(poly, joint, nextj);
        joint = nextj;
    }
    replace_curve(poly, joint, startj);
    poly.set_line_start(startj->get_next_line());
}

static clip_joint* strip_off(clip_joint* joint)
{
    assert(joint);
    auto t = joint->get_type();
    switch(t)
    {
    case ct_mirror_joint:
        return static_cast<clip_mirror_joint*>(joint)->get_mirror();
    case ct_final_joint:
        return static_cast<clip_final_joint*>(joint)->get_mirror();
    default:
        assert(!"unexpected.");
        return 0;
    }
}

void clip_sweep_line_algorithm::replace_curve(clip_polygon& poly, clip_joint*& src1, clip_joint*& src2)
{
    assert(src1 && src2);
    auto* joint1 = src1;
    auto* joint2 = src2;
    auto* m1 = strip_off(joint1);
    auto* m2 = strip_off(joint2);
    assert(m1 && m2);
    auto t = joint1->get_type();
    assert(t == ct_mirror_joint || t == ct_final_joint);
    auto* fj1 = (t == ct_final_joint) ? static_cast<clip_final_joint*>(joint1) :
        poly.create_final_joint(m1);
    clip_final_joint* fj2 = 0;
    if(joint1 == joint2)
        fj2 = fj1;
    else {
        t = joint2->get_type();
        assert(t == ct_mirror_joint || t == ct_final_joint);
        fj2 = (t == ct_final_joint) ? static_cast<clip_final_joint*>(joint2) :
            poly.create_final_joint(m2);
    }
    /* output the final joint */
    src1 = fj1;
    src2 = fj2;
    auto mt1 = m1->get_type();
    auto mt2 = m2->get_type();
    auto replace_line = [&fj1, &fj2, &joint1, &joint2]() {
        fj1->set_point(joint1->get_point());
        fj2->set_point(joint2->get_point());
        clip_connect(joint1->get_prev_line(), fj1);
        clip_connect(fj1, joint1->get_next_line(), fj2);
        clip_connect(fj2, joint2->get_next_line());
    };
    curve_spliter* csp = 0;
    const vec2* p1 = 0;
    const vec2* p2 = 0;
    if(mt1 == ct_end_joint && mt2 == ct_end_joint) {
        /* a straight line or a curve needn't be splitted. */
        auto* mj1 = static_cast<clip_end_joint*>(m1);
        auto* mj2 = static_cast<clip_end_joint*>(m2);
        auto* pnf11 = mj1->get_path_info(0);
        auto* pnf12 = mj1->get_path_info(1);
        auto* pnf21 = mj2->get_path_info(0);
        auto* pnf22 = mj2->get_path_info(1);
        auto* pnf = ((pnf11 == pnf21) || (pnf11 == pnf22)) ? pnf11 : pnf12;
        assert(pnf);
        int o = pnf->get_order();
        if(o == 1)
            replace_line();
        else if(o == 2) {
            vec2 cp[3];
            pnf->get_points(cp, _countof(cp));
            auto* c1 = poly.create_final_joint(0);
            assert(c1);
            c1->set_point(cp[1]);
            fj1->set_point(cp[0]);
            fj2->set_point(cp[2]);
            clip_connect(joint1->get_prev_line(), fj1);
            clip_connect(fj1, joint1->get_next_line(), fj2);
            poly.create_line(fj1, c1);
            poly.create_line(c1, fj2);
        }
        else if(o == 3) {
            vec2 cp[4];
            pnf->get_points(cp, _countof(cp));
            auto* c1 = poly.create_final_joint(0);
            auto* c2 = poly.create_final_joint(0);
            assert(c1 && c2);
            c1->set_point(cp[1]);
            c2->set_point(cp[2]);
            fj1->set_point(cp[0]);
            fj2->set_point(cp[3]);
            clip_connect(joint1->get_prev_line(), fj1);
            clip_connect(fj1, joint1->get_next_line(), fj2);
            poly.create_line(fj1, c1);
            poly.create_line(c1, c2);
            poly.create_line(c2, fj2);
        }
        else {
            assert(!"unexpected order of curve.");
            return;
        }
        return;
    }
    else if(mt1 == ct_intersect_joint && mt2 == ct_end_joint) {
        auto* mj1 = static_cast<clip_intersect_joint*>(m1);
        auto* mj2 = static_cast<clip_end_joint*>(m2);
        auto* pnf11 = retrieve_path_info(mj1->get_orient(0), mj1->get_orient(1));
        auto* pnf12 = retrieve_path_info(mj1->get_cut(0), mj1->get_cut(1));
        auto* pnf21 = mj2->get_path_info(0);
        auto* pnf22 = mj2->get_path_info(1);
        auto* pnf = ((pnf11 == pnf21) || (pnf11 == pnf22)) ? pnf11 : pnf12;
        assert(pnf);
        if(pnf->get_order() == 1)
            return replace_line();
        csp = _spliters.find(pnf)->second;
        p1 = &_fixed_points.find(m1)->second;
        p2 = &m2->get_point();
    }
    else if(mt1 == ct_end_joint && mt2 == ct_intersect_joint) {
        auto* mj1 = static_cast<clip_end_joint*>(m1);
        auto* mj2 = static_cast<clip_intersect_joint*>(m2);
        auto* pnf11 = retrieve_path_info(mj2->get_orient(0), mj2->get_orient(1));
        auto* pnf12 = retrieve_path_info(mj2->get_cut(0), mj2->get_cut(1));
        auto* pnf21 = mj1->get_path_info(0);
        auto* pnf22 = mj1->get_path_info(1);
        auto* pnf = ((pnf11 == pnf21) || (pnf11 == pnf22)) ? pnf11 : pnf12;
        assert(pnf);
        if(pnf->get_order() == 1)
            return replace_line();
        csp = _spliters.find(pnf)->second;
        p1 = &_fixed_points.find(m2)->second;
        p2 = &m1->get_point();
    }
    else {
        assert(mt1 == ct_intersect_joint && mt2 == ct_intersect_joint);
        auto* mj1 = static_cast<clip_intersect_joint*>(m1);
        auto* mj2 = static_cast<clip_intersect_joint*>(m2);
        auto* pnf11 = retrieve_path_info(mj1->get_orient(0), mj1->get_orient(1));
        auto* pnf12 = retrieve_path_info(mj1->get_cut(0), mj1->get_cut(1));
        auto* pnf21 = retrieve_path_info(mj2->get_orient(0), mj2->get_orient(1));
        auto* pnf22 = retrieve_path_info(mj2->get_cut(0), mj2->get_cut(1));
        auto* pnf = ((pnf11 == pnf21) || (pnf11 == pnf22)) ? pnf11 : pnf12;
        assert(pnf);
        if(pnf->get_order() == 1)
            return replace_line();
        csp = _spliters.find(pnf)->second;
        p1 = &_fixed_points.find(m1)->second;
        p2 = &_fixed_points.find(m2)->second;
    }
    assert(csp && p1 && p2);
    auto* fsp = curve_helper::query_spliter(csp, *p1, *p2);
    assert(fsp && fsp->is_leaf());
    clip_connect(joint1->get_prev_line(), fj1);
    clip_connect(fj2, joint2->get_next_line());
    vec2 cp[4];
    int cnt = fsp->get_point_count();
    fsp->get_points(cp, _countof(cp));
    assert(cnt == 3 || cnt == 4);
    vec2 d1, d2;
    d1.sub(cp[0], m1->get_point());
    d2.sub(cp[cnt - 1], m1->get_point());
    bool is_reversed = d1.lengthsq() > d2.lengthsq();
    if(cnt == 3) {
        auto* c1 = poly.create_final_joint(0);
        assert(c1);
        c1->set_point(cp[1]);
        if(!is_reversed) {
            fj1->set_point(cp[0]);
            fj2->set_point(cp[2]);
        }
        else {
            fj1->set_point(cp[2]);
            fj2->set_point(cp[0]);
        }
        poly.create_line(fj1, c1);
        poly.create_line(c1, fj2);
    }
    else {
        assert(cnt == 4);
        auto* c1 = poly.create_final_joint(0);
        auto* c2 = poly.create_final_joint(0);
        assert(c1 && c2);
        if(!is_reversed) {
            fj1->set_point(cp[0]);
            c1->set_point(cp[1]);
            c2->set_point(cp[2]);
            fj2->set_point(cp[3]);
        }
        else {
            fj1->set_point(cp[3]);
            c1->set_point(cp[2]);
            c2->set_point(cp[1]);
            fj2->set_point(cp[0]);
        }
        poly.create_line(fj1, c1);
        poly.create_line(c1, c2);
        poly.create_line(c2, fj2);
    }
}

//////////////////////////////////////////////////////////////////////////

void clip_assembler::install_sweepers(clip_sweep_line* line1, clip_sweep_line* line2)
{
    assert(line1 && line2);
    if(!_upside.empty())
        _upside.clear();
    if(!_downside.empty())
        _downside.clear();
    create_upside_sweepers(_upside, line1, line2);
    create_downside_sweepers(_downside, line2, line1);
    assert(_upside.size() == _downside.size());
}

static const clip_sweeper& get_assembly_sweeper(const clip_assembly_node& node)
{
    static clip_sweeper bad_sweeper;
    auto t = node.get_tag();
    switch(t)
    {
    case ant_sweeper:
        return *node.to_const_class1().get_up_sweeper();
    case ant_point:
        return *node.to_const_class2().get_sweeper();
    default:
        assert(!"unexpected assembly node.");
        return bad_sweeper;
    }
}

static const vec2& get_assembly_node_point(const clip_assembly_node& node)
{
    static vec2 bad_point;
    auto t = node.get_tag();
    switch(t)
    {
    case ant_sweeper:
        return node.to_const_class1().get_up_sweeper()->get_point();
    case ant_point:
        return node.to_const_class2().get_sweeper()->get_point();
    case ant_patch:
        return node.to_const_class3().get_point();
    default:
        assert(!"unexpected assembly node.");
        return bad_point;
    }
}

static clip_joint* get_assembly_bind_joint(const clip_assembly_node& node)
{
    if(node.get_tag() != ant_sweeper)
        return 0;
    auto& c1 = node.to_const_class1();
    auto* s = c1.get_up_sweeper();
    assert(s && s->joint);
    auto* sj = s->joint;
    if(sj->get_tag() != cst_endpoint)
        return 0;
    auto* ep = static_cast<clip_sweep_endpoint*>(sj);
    return ep->get_joint();
}

static clip_joint* get_assembly_dual_joint(const clip_assembly_node& node)
{
    if(node.get_tag() != ant_point)
        return 0;
    auto& c2 = node.to_const_class2();
    auto* s = c2.get_sweeper();
    assert(s && s->joint);
    auto* sj = s->joint;
    if(sj->get_tag() != cst_endpoint)
        return 0;
    auto* ep = static_cast<clip_sweep_endpoint*>(sj);
    return ep->get_joint();
}

static void emplace_assembly_node(clip_assembly_nodes& nodes, const clip_assembly_node& node)
{
    auto f = std::lower_bound(nodes.begin(), nodes.end(), node, [](const clip_assembly_node& node1, const clip_assembly_node& node2)->bool {
        auto& p1 = get_assembly_node_point(node1);
        auto& p2 = get_assembly_node_point(node2);
        assert(p1.y == p2.y);
        return p1.x < p2.x;
    });
    nodes.insert(f, node);
}

static void emplace_assembly_nodes(clip_assembly_nodes& nodes, clip_result_iter p)
{
    assert(p.is_valid());
    for(auto i = p.child(); i.is_valid(); i.to_next()) {
        if(!i->is_patch())
            continue;
        auto& patch = static_cast<clip_patch&>(*i);
        clip_assembly_node left(ant_point), right(ant_point);
        auto& lcls = left.to_class2();
        auto& rcls = right.to_class2();
        lcls.set_patch_iter(i);
        rcls.set_patch_iter(i);
        lcls.set_sweeper(&patch.get_end_point());
        rcls.set_sweeper(&patch.get_start_point());
        emplace_assembly_node(nodes, left);
        emplace_assembly_node(nodes, right);
        if(!i.is_leaf())
            emplace_assembly_nodes(nodes, i);
    }
}

void clip_assembler::prepare_assembly_nodes()
{
    if(!_nodes.empty())
        _nodes.clear();
    /* 1.create assembly nodes from sweepers */
    assert(_upside.size() == _downside.size());
    auto i = _upside.begin();
    auto j = _downside.begin();
    auto end = _upside.end();
    for(; i != end; ++ i, ++ j) {
        clip_assembly_node node(ant_sweeper);
        auto& sweeper = node.to_class1();
        sweeper.set_up_sweeper(&(*i));
        sweeper.set_down_sweeper(&(*j));
        _nodes.push_back(node);
    }
    /* 2.emplace the patches end points to the suitable position */
    auto r = _result.get_root();
    assert(r.is_valid());
    emplace_assembly_nodes(_nodes, r);
}

clip_result_iter clip_assembler::close_patch(iterator i, const clip_assembly_sweeper_node& node1, const clip_assembly_sweeper_node& node2)
{
    assert(i && i->is_patch());
    auto n = _result.insert<clip_polygon>(i);
    auto& patch = static_cast<clip_patch&>(*i);
    assert(is_neighbour_joint(node1.get_down_sweeper()->joint, node2.get_down_sweeper()->joint));
    auto& left_o = patch.get_end_point();
    auto& right_o = patch.get_start_point();
    auto& left_u = *node1.get_up_sweeper();
    auto& right_u = *node2.get_up_sweeper();
    auto& left_d = *node1.get_down_sweeper();
    auto& right_d = *node2.get_down_sweeper();
    if(left_o.joint != left_u.joint) {
        auto* p = patch.create_assumed_mj(left_u);
        patch.fix_line_back(patch.get_line_end(), p);
    }
    if(right_o.joint != right_u.joint) {
        auto* p = patch.create_assumed_mj(right_u);
        patch.fix_line_front(patch.get_line_start(), p);
    }
    clip_mirror_joint* left = 0;
    clip_mirror_joint* right = 0;
    if(left_d.get_point() == right_d.get_point())
        left = right = patch.create_assumed_mj(left_d);
    else {
        left = patch.create_assumed_mj(left_d);
        right = patch.create_assumed_mj(right_d);
        patch.create_line(left, right);
    }
    assert(left && right);
    auto* line1 = patch.fix_line_back(patch.get_line_end(), left);
    auto* line2 = patch.fix_line_front(patch.get_line_start(), right);
    assert(line1 && line2);
    patch.set_line_end(line1);
    patch.set_line_start(line2);
    n->convert_from(patch);
    n.get_wrapper()->swap_children(*i.get_wrapper());
    _result.erase(i);
    return n;
}

clip_result_iter clip_assembler::close_patch(iterator i, iterator j)
{
    assert(i && i->is_patch() && j && j->is_patch());
    auto n = _result.insert<clip_patch>(i);
    auto& patch1 = static_cast<clip_patch&>(*i);
    auto& patch2 = static_cast<clip_patch&>(*j);
    assert(is_neighbour_joint(patch1.get_start_point().joint, patch2.get_start_point().joint) &&
        is_neighbour_joint(patch1.get_end_point().joint, patch2.get_end_point().joint)
        );
    patch2.reverse_direction();
    patch1.adopt(patch2);
    auto* line1 = patch1.get_line_end();
    auto* line2 = patch2.get_line_start();
    auto* line3 = patch2.get_line_end();
    auto* line4 = patch1.get_line_start();
    assert(line1 && line2 && line3 && line4);
    (line1->get_point(1) == line2->get_point(0)) ? clip_connect(line1->get_joint(1), line2) :
        patch1.create_line(line1->get_joint(1), line2->get_joint(0));
    (line3->get_point(1) == line4->get_point(0)) ? clip_connect(line3->get_joint(1), line4) :
        patch1.create_line(line3->get_joint(1), line4->get_joint(0));
    for(auto c = j.child(); c.is_valid();) {
        auto next = c.next();
        clip_result t;
        _result.detach(t, c);
        _result.attach(t, _result.insert(i));
        c = next;
    }
    n->convert_from(patch1);
    _result.erase(j);
    n.get_wrapper()->swap_children(*i.get_wrapper());
    _result.erase(i);
    return n;
}

void clip_assembler::merge_patch_parallel(iterator piter1, iterator piter2, clip_sweeper& sweeper1, clip_sweeper& sweeper2)
{
    assert(piter1 && piter2);
    assert(piter1->is_patch() && piter2->is_patch());
    auto& patch1 = static_cast<clip_patch&>(*piter1);
    auto& patch2 = static_cast<clip_patch&>(*piter2);
    assert(&patch1.get_start_point() == &sweeper1 &&
        &patch2.get_end_point() == &sweeper2
        );
    patch1.adopt(patch2);
    auto* line2 = patch1.get_line_start();
    auto* line1 = patch2.get_line_end();
    assert(line2 && line1);
    (sweeper1.get_point() == sweeper2.get_point()) ? clip_connect(line1->get_joint(1), line2) :
        patch1.create_line(line1->get_joint(1), line2->get_joint(0));
    patch1.set_line_start(patch2.get_line_start());
    patch1.set_start_point(patch2.get_start_point());
    for(auto c = piter2.child(); c.is_valid();) {
        auto next = c.next();
        clip_result t;
        _result.detach(t, c);
        _result.attach(t, _result.birth_tail(piter1));
        c = next;
    }
    _result.erase(piter2);
}

void clip_assembler::merge_patch_head(iterator piter1, iterator piter2, clip_sweeper& sweeper1, clip_sweeper& sweeper2)
{
    assert(piter1 && piter2);
    assert(piter1->is_patch() && piter2->is_patch());
    auto& patch1 = static_cast<clip_patch&>(*piter1);
    auto& patch2 = static_cast<clip_patch&>(*piter2);
    assert(&patch1.get_end_point() == &sweeper1 &&
        &patch2.get_end_point() == &sweeper2
        );
    patch2.reverse_direction();
    patch1.adopt(patch2);
    auto* line1 = patch1.get_line_end();
    auto* line2 = patch2.get_line_start();
    assert(line1 && line2);
    (sweeper1.get_point() == sweeper2.get_point()) ? clip_connect(line1->get_joint(1), line2) :
        patch1.create_line(line1->get_joint(1), line2->get_joint(0));
    patch1.set_line_end(patch2.get_line_end());
    patch1.set_end_point(patch2.get_end_point());
    for(auto c = piter2.child(); c.is_valid();) {
        auto next = c.next();
        clip_result t;
        _result.detach(t, c);
        _result.attach(t, _result.insert(piter1));
        c = next;
    }
    _result.erase(piter2);
}

void clip_assembler::merge_patch_tail(iterator piter1, iterator piter2, clip_sweeper& sweeper1, clip_sweeper& sweeper2)
{
    assert(piter1 && piter2);
    assert(piter1->is_patch() && piter2->is_patch());
    auto& patch1 = static_cast<clip_patch&>(*piter1);
    auto& patch2 = static_cast<clip_patch&>(*piter2);
    assert(&patch1.get_start_point() == &sweeper1 &&
        &patch2.get_start_point() == &sweeper2
        );
    patch2.reverse_direction();
    patch1.adopt(patch2);
    auto* line1 = patch2.get_line_end();
    auto* line2 = patch1.get_line_start();
    assert(line1 && line2);
    (sweeper1.get_point() == sweeper2.get_point()) ? clip_connect(line1->get_joint(1), line2) :
        patch1.create_line(line1->get_joint(1), line2->get_joint(0));
    patch1.set_line_start(patch2.get_line_start());
    patch1.set_start_point(patch2.get_start_point());
    auto ipos = piter1;
    for(auto c = piter2.child(); c.is_valid();) {
        auto next = c.next();
        clip_result t;
        _result.detach(t, c);
        _result.attach(t, _result.insert_after(ipos));
        ipos.to_next();
        c = next;
    }
    _result.erase(piter2);
}

static bool is_sample_joint(clip_joint* joint)
{
    assert(joint);
    auto t = joint->get_type();
    switch(t)
    {
    case ct_end_joint:
    case ct_interpolate_joint:
        return true;
    case ct_intersect_joint:
    case ct_pivot_joint:
        return false;
    case ct_mirror_joint:
    case ct_final_joint:
        return is_sample_joint(strip_off(joint));
    default:
        assert(!"unexpected.");
        return false;
    }
}

static bool is_point_inside_polygon(clip_polygon* poly, const vec2& pt)
{
    assert(poly);
    auto is_xplus_clipped = [](clip_line* line, const vec2& pt)->bool {
        assert(line);
        auto* j1 = line->get_joint(0);
        auto* j2 = line->get_joint(1);
        assert(j1 && j2);
        auto& p1 = j1->get_point();
        auto& p2 = j2->get_point();
        if((pt.y > p1.y && pt.y > p2.y) || (pt.y < p1.y && pt.y < p2.y))
            return false;
        vec2 d;
        vec3 coef;
        d.sub(p2, p1);
        get_linear_coefficient(coef, p1, d);
        if(fuzzy_zero(coef.x))
            return fuzzy_zero(pt.y - p1.y);
        float x = (-coef.z - coef.y * pt.y) / coef.x;
        return pt.x > x;
    };
    int c = 0;
    auto* line1 = poly->get_line_start();
    assert(line1);
    if(is_xplus_clipped(line1, pt))
        c ++;
    for(auto* line = line1->get_next_line(); line != line1; line = line->get_next_line()) {
        assert(line);
        if(is_xplus_clipped(line, pt))
            c ++;
    }
    return c % 2 == 1;
}

static bool is_polygon_inside_polygon(clip_polygon* poly, clip_polygon* inner)
{
    assert(poly && inner && !poly->is_patch() && !inner->is_patch());
    auto* line1 = inner->get_line_start();
    assert(line1);
    auto* p1 = line1->get_joint(0);
    assert(p1);
    if(is_sample_joint(p1))
        return is_point_inside_polygon(poly, p1->get_point());
    auto* p2 = line1->get_joint(1);
    assert(p2);
    if(is_sample_joint(p2))
        return is_point_inside_polygon(poly, p2->get_point());
    vec2 p;
    p.add(p1->get_point(), p2->get_point());
    p.scale(0.5f);
    return is_point_inside_polygon(poly, p);
}

void clip_assembler::split_patch(iterator piter, clip_joint* sp1, clip_joint* sp2)
{
    assert(piter && sp1 && sp2);
    assert(piter->is_patch());
    auto& patch = static_cast<clip_patch&>(*piter);
    auto* line1 = patch.get_line_start();
    auto* line2 = patch.get_line_end();
    assert(line1 && line2);
    auto* sp1_prev = sp1->get_prev_line();
    auto* sp1_next = sp1->get_next_line();
    auto* sp2_prev = sp2->get_prev_line();
    auto* sp2_next = sp2->get_next_line();
    assert(sp2_prev && sp1_next);
    assert(sp1_prev || sp2_next);
    auto i = _result.birth<clip_polygon>(piter.parent());
    assert(i);
    auto& poly = *i;
    clip_connect(sp2_prev, sp1);
    clip_connect(sp1_prev, sp2);
    poly.set_line_start(sp1->get_next_line());
    if(sp1_prev && !sp2_next)
        patch.set_line_end(sp1_prev);
    else if(!sp1_prev && sp2_next)
        patch.set_line_start(sp2_next);
    for(auto j = piter.child(); j;) {
        auto k = j.next();
        if(!j->is_patch() && is_polygon_inside_polygon(&poly, &*j)) {
            clip_result t;
            _result.detach(t, j);
            _result.attach(t, _result.birth_tail(i));
        }
        j = k;
    }
}

static bool is_same_joint(clip_joint* joint1, clip_joint* joint2)
{
    assert(joint1 && joint2);
    auto t1 = joint1->get_type();
    auto t2 = joint2->get_type();
    if(t1 == ct_intersect_joint && t2 == ct_intersect_joint) {
        auto* j1 = static_cast<clip_intersect_joint*>(joint1);
        auto* j2 = static_cast<clip_intersect_joint*>(joint2);
        return j1 == j2 || j1->get_symmetric() == j2;
    }
    return false;
}

bool clip_assembler::try_split_patch(iterator piter)
{
    assert(piter && piter->is_patch());
    auto& patch = static_cast<clip_patch&>(*piter);
    auto* line1 = patch.get_line_start();
    auto* line2 = patch.get_line_end();
    assert(line1 && line2);
    vector<clip_joint*> sel_joints;
    auto is_select_joint = [](clip_joint* p)->bool {
        assert(p);
        auto* m = strip_off(p);
        if(m == 0)
            return false;
        auto t = m->get_type();
        return t == ct_intersect_joint || t == ct_pivot_joint;
    };
    if(auto* joint = line1->get_joint(0)) {
        if(is_select_joint(joint))
            sel_joints.push_back(joint);
    }
    for(auto* line = line1; line != line2; line = line->get_next_line()) {
        auto* joint = line->get_joint(1);
        assert(joint);
        if(is_select_joint(joint))
            sel_joints.push_back(joint);
    }
    if(auto* joint = line2->get_joint(1)) {
        if(is_select_joint(joint))
            sel_joints.push_back(joint);
    }
    int cap = (int)sel_joints.size();
    if(cap <= 1)
        return false;
    clip_joint *sp1 = 0, *sp2 = 0;
    for(int c = 0; c < cap; c ++) {
        sp1 = sel_joints.at(c);
        for(int i = c + 1; i < cap; i ++) {
            auto* j = sel_joints.at(i);
            if(is_same_joint(strip_off(sp1), strip_off(j))) {
                sp2 = j;
                break;
            }
        }
        if(sp2 != 0)
            break;
    }
    if(!sp2)
        return false;
    assert(sp1 && sp2);
    split_patch(piter, sp1, sp2);
    return true;
}

void clip_assembler::create_patch(clip_patch& patch, const clip_sweeper& up1, const clip_sweeper& up2, const clip_sweeper& down1, const clip_sweeper& down2)
{
    clip_mirror_joint *t1, *t2, *b1, *b2;
    t1 = patch.create_assumed_mj(up1);
    if(up1.joint == up2.joint)
        t2 = t1;
    else {
        t2 = patch.create_assumed_mj(up2);
        patch.create_line(t2, t1);
    }
    b1 = patch.create_assumed_mj(down1);
    b2 = (down1.joint == down2.joint) ? b1 : patch.create_assumed_mj(down2);
    assert(t1 && t2);
    auto* line1 = patch.create_line(b2, t2);
    auto* line2 = patch.create_line(t1, b1);
    patch.set_line_start(line1);
    patch.set_line_end(line2);
    patch.set_start_point(down2);
    patch.set_end_point(down1);
}

void clip_assembler::proceed_patch(clip_patch& patch, const clip_assembly_sweeper_node& node1, const clip_assembly_sweeper_node& node2)
{
    auto& left_o = patch.get_end_point();
    auto& right_o = patch.get_start_point();
    auto& left_u = *node1.get_up_sweeper();
    auto& right_u = *node2.get_up_sweeper();
    auto& left_d = *node1.get_down_sweeper();
    auto& right_d = *node2.get_down_sweeper();
    if(left_o.joint != left_u.joint) {
        auto* p = patch.create_assumed_mj(left_u);
        patch.fix_line_back(patch.get_line_end(), p);
    }
    if(right_o.joint != right_u.joint) {
        auto* p = patch.create_assumed_mj(right_u);
        patch.fix_line_front(patch.get_line_start(), p);
    }
    auto* left = patch.create_assumed_mj(left_d);
    auto* right = patch.create_assumed_mj(right_d);
    auto* line1 = patch.fix_line_back(patch.get_line_end(), left);
    auto* line2 = patch.fix_line_front(patch.get_line_start(), right);
    patch.set_line_end(line1);
    patch.set_line_start(line2);
    patch.set_start_point(right_d);
    patch.set_end_point(left_d);
}

clip_assembler_exclude::clip_assembler_exclude(clip_result& result):
    clip_assembler(result)
{
}

void clip_assembler_exclude::proceed(clip_sweep_lines& sweeplines)
{
    /* feed an empty root to achieve the multi top entries. */
    assert(!_result.is_valid());
    auto r = _result.birth<clip_polygon>(clip_result_iter(0));
    assert(r);
    assert(sweeplines.size() >= 2);
    auto i = sweeplines.begin();
    auto j = std::next(i);
    auto end = sweeplines.end();
    for(; j != end; i = j ++)
        proceed(*i, *j);
}

void clip_assembler_exclude::proceed(clip_sweep_line* line1, clip_sweep_line* line2)
{
    assert(line1 && line2);
    install_sweepers(line1, line2);
    prepare_assembly_nodes();
    proceed_patches();
    proceed_sweepers();
    finish_proceed_sub_patches(_result.get_root());
}

static clip_assembly_nodes::iterator find_assembly_node(clip_assembly_nodes& nodes, clip_patch* patch, clip_sweeper* sweeper)
{
    assert(patch && sweeper);
    float x = sweeper->get_point().x;
    auto f = std::lower_bound(nodes.begin(), nodes.end(), x, [](const clip_assembly_node& node, float x)->bool {
        auto& p = get_assembly_node_point(node);
        return p.x < x;
    });
    if(f == nodes.end())
        return f;
    if(f->get_tag() == ant_point) {
        auto& cp =  f->to_class2();
        auto iter = cp.get_patch_iter();
        assert(iter && iter->is_patch());
        if(static_cast<clip_patch*>(&*iter) == patch)
            return f;
    }
    /* test the codes below! */
    auto f1 = f, f2 = f;
    for(; f1 != nodes.begin() && get_assembly_node_point(*f1).x == x; -- f1);
    for(; f2 != nodes.end() && get_assembly_node_point(*f2).x == x; ++ f2);
    if(get_assembly_node_point(*f1).x != x)
        ++ f1;
    for(auto i = f1; i != f2; ++ i) {
        if(i->get_tag() == ant_point) {
            auto& cp = i->to_class2();
            auto iter = cp.get_patch_iter();
            assert(iter && iter->is_patch());
            if(static_cast<clip_patch*>(&*iter) == patch)
                return i;
        }
    }
    assert(!"unexpected situation in find assembly node.");
    return nodes.end();
}

static clip_assembly_nodes::iterator find_assembly_related_node(clip_assembly_nodes& nodes, clip_assembly_nodes::iterator i)
{
    assert(i != nodes.end());
    assert(i->get_tag() == ant_point);
    auto& p_node = i->to_class2();
    auto* sweeper = p_node.get_sweeper();
    assert(sweeper);
    float x = sweeper->get_point().x;
    auto f1 = i, f2 = i;
    for(; f1 != nodes.begin() && get_assembly_node_point(*f1).x == x; -- f1);
    for(; f2 != nodes.end() && get_assembly_node_point(*f2).x == x; ++ f2);
    if(get_assembly_node_point(*f1).x != x)
        ++ f1;
    for(auto j = f1; j != f2; ++ j) {
        if(j->get_tag() == ant_sweeper)
            return j;
    }
    /* maybe there was a horizontal linkage. */
    auto* sj = sweeper->joint;
    if(sj->get_tag() == cst_endpoint) {
        auto* ep = static_cast<clip_sweep_endpoint*>(sj);
        auto* oj = ep->get_joint();
        assert(oj);
        auto* prevj = oj->get_prev_joint();
        auto* nextj = oj->get_next_joint();
        assert(prevj && nextj);
        clip_joint* f = 0;
        if(prevj->get_point().y == oj->get_point().y)
            f = prevj;
        else if(nextj->get_point().y == oj->get_point().y)
            f = nextj;
        if(f) {
            float x = f->get_point().x;
            auto fd = std::lower_bound(nodes.begin(), nodes.end(), x, [](const clip_assembly_node& node, float x)->bool {
                auto& p = get_assembly_node_point(node);
                return p.x < x;
            });
            if(fd != nodes.end()) {
                auto f1 = fd, f2 = fd;
                for(; f1 != nodes.begin() && get_assembly_node_point(*f1).x == x; -- f1);
                for(; f2 != nodes.end() && get_assembly_node_point(*f2).x == x; ++ f2);
                if(get_assembly_node_point(*f1).x != x)
                    ++ f1;
                for(auto j = f1; j != f2; ++ j) {
                    if(get_assembly_bind_joint(*j) == f)
                        return j;
                }
            }
        }
    }
    return nodes.end();
}

static clip_assembly_nodes::iterator find_assembly_dual_node(clip_assembly_nodes& nodes, clip_assembly_nodes::iterator i)
{
    assert(i != nodes.end());
    assert(i->get_tag() == ant_point);
    auto& p_node = i->to_class2();
    auto* sweeper = p_node.get_sweeper();
    assert(sweeper);
    float x = sweeper->get_point().x;
    auto f1 = i, f2 = i;
    for(; f1 != nodes.begin() && get_assembly_node_point(*f1).x == x; -- f1);
    for(; f2 != nodes.end() && get_assembly_node_point(*f2).x == x; ++ f2);
    if(get_assembly_node_point(*f1).x != x)
        ++ f1;
    for(auto j = f1; j != f2; ++ j) {
        if(j != i && j->get_tag() == ant_point)
            return j;
    }
    auto* sj = sweeper->joint;
    if(sj->get_tag() == cst_endpoint) {
        auto* ep = static_cast<clip_sweep_endpoint*>(sj);
        auto* oj = ep->get_joint();
        assert(oj);
        auto* prevj = oj->get_prev_joint();
        auto* nextj = oj->get_next_joint();
        assert(prevj && nextj);
        clip_joint* f = 0;
        if(prevj->get_point().y == oj->get_point().y)
            f = prevj;
        else if(nextj->get_point().y == oj->get_point().y)
            f = nextj;
        if(f) {
            float x = f->get_point().x;
            auto fd = std::lower_bound(nodes.begin(), nodes.end(), x, [](const clip_assembly_node& node, float x)->bool {
                auto& p = get_assembly_node_point(node);
                return p.x < x;
            });
            if(fd != nodes.end()) {
                auto f1 = fd, f2 = fd;
                for(; f1 != nodes.begin() && get_assembly_node_point(*f1).x == x; -- f1);
                for(; f2 != nodes.end() && get_assembly_node_point(*f2).x == x; ++ f2);
                if(get_assembly_node_point(*f1).x != x)
                    ++ f1;
                for(auto j = f1; j != f2; ++ j) {
                    if(j->get_tag() == ant_point) {
                        auto& sp = j->to_const_class2();
                        if(sp.get_patch_iter() != p_node.get_patch_iter() && get_assembly_dual_joint(*j) == f)
                            return j;
                    }
                }
            }
        }
    }
    return nodes.end();
}

static bool is_patch_closed(const clip_assembly_sweeper_node& node1, const clip_assembly_sweeper_node& node2)
{
    auto* s1 = node1.get_down_sweeper();
    auto* s2 = node2.get_down_sweeper();
    assert(s1 && s2);
    return is_neighbour_joint(s1->joint, s2->joint);
}

void clip_assembler_exclude::proceed_patches()
{
    auto i = _nodes.begin();
    auto end = _nodes.end();
    while(i != end) {
        if(i->get_tag() != ant_point) {
            ++ i;
            continue;
        }
        auto& left = i->to_class2();
        auto patch_iter = left.get_patch_iter();
        assert(patch_iter && patch_iter->is_patch());
        auto* patch = static_cast<clip_patch*>(&*patch_iter);
        assert(patch && (&patch->get_end_point() == left.get_sweeper()));
        auto f = find_assembly_node(_nodes, patch, &patch->get_start_point());
        auto sweeper_l = find_assembly_related_node(_nodes, i);
        auto sweeper_r = find_assembly_related_node(_nodes, f);
        assert(sweeper_l != _nodes.end());
        if(sweeper_r == _nodes.end()) {
            auto dual = find_assembly_dual_node(_nodes, f);
            assert(dual != _nodes.end());
            auto& c2 = dual->to_class2();
            auto* patch2 = static_cast<clip_patch*>(&*c2.get_patch_iter());
            auto m = find_assembly_node(_nodes, patch2, &patch2->get_start_point());
            assert(m != _nodes.end() && m->get_tag() == ant_point);
            merge_patch(f->to_const_class2(), dual->to_const_class2());
            _nodes.erase(f);
            _nodes.erase(dual);
            m->to_class2().set_patch_iter(patch_iter);
            continue;
        }
        auto& s1 = sweeper_l->to_const_class1();
        auto& s2 = sweeper_r->to_const_class1();
        if(is_patch_closed(s1, s2))
            close_patch(patch_iter, s1, s2);
        else {
            proceed_patch(*patch, s1, s2);
            /* repair nodes */
            clip_assembly_node node1(ant_patch), node2(ant_patch);
            auto& l = node1.to_class3();
            auto& r = node2.to_class3();
            l.set_patch_iter(patch_iter);
            r.set_patch_iter(patch_iter);
            l.set_point(get_assembly_node_point(*sweeper_l));
            r.set_point(get_assembly_node_point(*sweeper_r));
            _nodes.insert(sweeper_l, node1);
            _nodes.insert(sweeper_r, node2);
        }
        auto next = _nodes.erase(i);
        (next == f) ? (next = _nodes.erase(f)): _nodes.erase(f);
        (next == sweeper_l) ? (next = _nodes.erase(sweeper_l)) : _nodes.erase(sweeper_l);
        (next == sweeper_r) ? (next = _nodes.erase(sweeper_r)) : _nodes.erase(sweeper_r);
        i = next;
    }
}

void clip_assembler_exclude::proceed_sweepers()
{
    if(!_iter_st.empty())
        _iter_st.swap(iterator_stack());
    if(_nodes.empty())
        return;
    _last_sib = iterator(0);
    auto r = _result.get_root();
    assert(r);
    _iter_st.push(r);
    auto i = _nodes.begin();
    auto end = _nodes.end();
    while(i != end) {
        auto t = i->get_tag();
        assert(t != ant_point);
        if(t == ant_patch) {
            auto& p = i->to_class3();
            auto patch_iter = p.get_patch_iter();
            assert(patch_iter);
            if(_iter_st.top() == patch_iter) {
                _last_sib = patch_iter;
                _iter_st.pop();
            }
            else {
                _last_sib = iterator(0);
                _iter_st.push(patch_iter);
            }
            ++ i;
            continue;
        }
        assert(t == ant_sweeper);
        auto j = std::next(i);
        assert(j != end && (j->get_tag() == ant_sweeper));
        auto parent = _iter_st.top();
        auto p = _last_sib ? _result.insert_after<clip_patch>(_last_sib) : _result.birth_tail<clip_patch>(parent);
        assert(p && p->is_patch());
        auto& patch = static_cast<clip_patch&>(*p);
        auto& s1 = i->to_class1();
        auto& s2 = j->to_class1();
        create_patch(patch, *s1.get_up_sweeper(), *s2.get_up_sweeper(), *s1.get_down_sweeper(), *s2.get_down_sweeper());
        i = std::next(j);
    }
}

static clip_joint* detect_ring_joint(clip_patch& patch)
{
    auto* line1 = patch.get_line_start();
    auto* line2 = patch.get_line_end();
    assert(line1 && line2);
    if(line1 == line2)
        return 0;
    auto* joint1 = line1->get_joint(0);
    if(!joint1) {
        line1 = line1->get_next_line();
        if(line1 == line2)
            return 0;
        joint1 = line1->get_joint(0);
    }
    assert(joint1);
    auto* joint2 = line2->get_joint(1);
    if(!joint2) {
        line2 = line2->get_prev_line();
        if(line1 == line2)
            return 0;
        joint2 = line2->get_joint(1);
    }
    assert(joint2);
    auto& p1 = joint1->get_point();
    auto& p2 = joint2->get_point();
    for(auto* line = line1; line != line2; line = line->get_next_line()) {
        assert(line);
        auto* joint = line->get_joint(1);
        assert(joint);
        auto& p = joint->get_point();
        if(p == p1 || p == p2)
            return joint;
    }
    return 0;
}

void clip_assembler_exclude::finish_proceed_patches(iterator p)
{
    assert(p.is_valid());
    if(!p->is_patch())
        return;
    finish_proceed_sub_patches(p);
    if(p.is_leaf()) {
        /* finish leaf patch if needed */
        assert(p->is_patch());
        static_cast<clip_patch&>(*p).finish_patch();
        return;
    }
    auto& patch = static_cast<clip_patch&>(*p);
    auto c1 = p.child();
    auto c2 = p.last_child();
    assert(c1 && c2);
    bool c1_is_patch = c1->is_patch();
    bool c2_is_patch = c2->is_patch();
    if(!c1_is_patch && !c2_is_patch)
        return;
    if(!c1_is_patch) {
        auto& patch2 = static_cast<clip_patch&>(*c2);
        if(is_neighbour_joint(patch.get_start_point().joint, patch2.get_start_point().joint)) {
            merge_patch_tail(p, c2, patch.get_start_point(), patch2.get_start_point());
            try_split_patch(p);
        }
    }
    else if(!c2_is_patch) {
        auto& patch1 = static_cast<clip_patch&>(*c1);
        if(is_neighbour_joint(patch.get_end_point().joint, patch1.get_end_point().joint)) {
            merge_patch_head(p, c1, patch.get_end_point(), patch1.get_end_point());
            try_split_patch(p);
        }
    }
    else {
        auto& patch1 = static_cast<clip_patch&>(*c1);
        auto& patch2 = static_cast<clip_patch&>(*c2);
        bool p1_linked = is_neighbour_joint(patch.get_end_point().joint, patch1.get_end_point().joint);
        bool p2_linked = is_neighbour_joint(patch.get_start_point().joint, patch2.get_start_point().joint);
        if(p1_linked && p2_linked) {
            if(c1 == c2)
                close_patch(p, c1);
            else {
                merge_patch_head(p, c1, patch.get_end_point(), patch1.get_end_point());
                merge_patch_tail(p, c2, patch.get_start_point(), patch2.get_start_point());
            }
        }
        else if(p1_linked) {
            merge_patch_head(p, c1, patch.get_end_point(), patch1.get_end_point());
            try_split_patch(p);
        }
        else if(p2_linked) {
            merge_patch_tail(p, c2, patch.get_start_point(), patch2.get_start_point());
            try_split_patch(p);
        }
    }
}

void clip_assembler_exclude::finish_proceed_sub_patches(iterator p)
{
    assert(p.is_valid());
    for(auto c = p.child(); c.is_valid(); c.to_next()) {
        if(c->is_patch())
            finish_proceed_patches(c);
    }
    for(auto c = p.child(); c.is_valid();) {
        if(!c->is_patch()) {
            c.to_next();
            continue;
        }
        auto c2 = c.next();
        if(!c2)
            break;
        if(!c2->is_patch()) {
            c = c2.next();
            continue;
        }
        auto& patch1 = static_cast<clip_patch&>(*c);
        auto& patch2 = static_cast<clip_patch&>(*c2);
        auto& sweeper1 = patch1.get_start_point();
        auto& sweeper2 = patch2.get_end_point();
        is_neighbour_joint(sweeper1.joint, sweeper2.joint) ? merge_patch_parallel(c, c2, sweeper1, sweeper2) : c = c2;
    }
}

//////////////////////////////////////////////////////////////////////////

void clip_test(const painter_path& path)
{
//     painter_path prepare;
//     painter_helper::close_sub_paths(prepare, path);
//     clip_polygons polygons;
//     clip_create_polygons(polygons, prepare);
//     for(auto* p : polygons) {
//         assert(p);
//         p->trace_segments();
//     }
//     for(auto* p : polygons) {
//         clip_sweep_line_algorithm sla;
//         sla.add_polygon(p);
//         sla.proceed();
//         //sla.tracing();
//         sla.trace_intersections();
//         //sla.trace_cells();
//         
//         clip_result result;
//         sla.output(result);
// 
//         trace(_t("@!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"));
//         result.for_each([](clip_polygon* poly) {
//             assert(poly && !poly->is_patch());
//             poly->trace_final();
//         });
// 
//         __asm nop;
//     }

    clip_polygons polys;
    clip_result result;
    painter_path output;
    clip_create_polygons(polys, path);
    clip_exclude(result, polys);
    clip_compile_path(output, result);
    output.tracing();
}

void clip_create_polygons(clip_polygons& polygons, const painter_path& path)
{
    int cap = path.size();
    if(!cap)
        return;
    for(int i = 0; i < cap;) {
        clip_polygon* polygon = gs_new(clip_polygon);
        polygons.push_back(polygon);
        i = polygon->create(path, i);
    }
}

static void compile_path(painter_path& path, const clip_result& poly_result, clip_result::const_iterator p)
{
    assert(p);
    auto compile_line_segment = [](painter_path& path, clip_line* line)->clip_line* {
        assert(line);
        auto* joint1 = line->get_joint(1);
        assert(joint1 && (joint1->get_type() == ct_final_joint));
        auto* fj1 = static_cast<clip_final_joint*>(joint1);
        if(!fj1->is_control_point()) {
            path.line_to(fj1->get_point());
            return line->get_next_line();
        }
        auto* line2 = line->get_next_line();
        assert(line2);
        auto* joint2 = line2->get_joint(1);
        assert(joint2 && (joint2->get_type() == ct_final_joint));
        auto* fj2 = static_cast<clip_final_joint*>(joint2);
        if(!fj2->is_control_point()) {
            path.quad_to(fj1->get_point(), fj2->get_point());
            return line2->get_next_line();
        }
        auto* line3 = line2->get_next_line();
        assert(line3);
        auto* joint3 = line3->get_joint(1);
        assert(joint3 && (joint3->get_type() == ct_final_joint));
        auto* fj3 = static_cast<clip_final_joint*>(joint3);
        assert(!fj3->is_control_point());
        path.cubic_to(fj1->get_point(), fj2->get_point(), fj3->get_point());
        return line3->get_next_line();
    };
    auto& poly = *p;
    //assert(!poly.is_patch());
    auto* line1 = poly.get_line_start();
    assert(line1);
    auto& p1 = line1->get_point(0);
    path.move_to(p1);
    auto* line = compile_line_segment(path, line1);
    while(line != line1)
        line = compile_line_segment(path, line);
    for(auto i = p.child(); i; i = i.next())
        compile_path(path, poly_result, i);
}

void clip_compile_path(painter_path& path, const clip_result& poly_result)
{
    auto p = poly_result.get_root();
    assert(p && !p.is_leaf());
    for(auto i = p.child(); i; i = i.next())
        compile_path(path, poly_result, i);
}

void clip_union(clip_result& poly_result, clip_polygons& polygons)
{
    clip_sweep_line_algorithm sla;
    sla.set_clip_tag(cp_union);
    for(auto* p : polygons)
        sla.add_polygon(p);
    sla.proceed();
    sla.output(poly_result);
}

void clip_intersect(clip_result& poly_result, clip_polygons& polygons)
{
    clip_sweep_line_algorithm sla;
    sla.set_clip_tag(cp_intersect);
    for(auto* p : polygons)
        sla.add_polygon(p);
    sla.proceed();
    sla.output(poly_result);
}

void clip_exclude(clip_result& poly_result, clip_polygons& polygons)
{
    clip_sweep_line_algorithm sla;
    sla.set_clip_tag(cp_exclude);
    for(auto* p : polygons)
        sla.add_polygon(p);
    sla.proceed();
    sla.output(poly_result);
}

__pink_end__
