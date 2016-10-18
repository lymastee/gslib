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

#include <ariel/batch.h>
#include <pink/utility.h>

__ariel_begin__

bat_triangle::bat_triangle()
{
    _joints[0] = _joints[1] = _joints[2] = 0;
    _is_reduced = false;
}

bat_triangle::bat_triangle(lb_joint* i, lb_joint* j, lb_joint* k)
{
    _joints[0] = i;
    _joints[1] = j;
    _joints[2] = k;
    _is_reduced = false;
}

void bat_triangle::make_rect(rectf& rc)
{
    float min_x = FLT_MAX, max_x = -FLT_MAX, min_y = FLT_MAX, max_y = -FLT_MAX;
    auto check_bound = [&](lb_joint* p) {
        if(p) {
            const auto& pt = p->get_point();
            min_x = gs_min(min_x, pt.x);
            min_y = gs_min(min_y, pt.y);
            max_x = gs_max(max_x, pt.x);
            max_y = gs_max(max_y, pt.y);
        }
    };
    check_bound(_joints[0]);
    check_bound(_joints[1]);
    check_bound(_joints[2]);
    rc.set_ltrb(min_x, min_y, max_x, max_y);
}

bat_type bat_triangle::decide() const
{
    assert(_joints[0] && _joints[1] && _joints[2]);
    auto t1 = _joints[0]->get_type();
    auto t2 = _joints[1]->get_type();
    auto t3 = _joints[2]->get_type();
    if((t1 == lbt_end_joint) && (t2 == lbt_end_joint) && (t3 == lbt_end_joint))
        return bf_cr;
    lb_joint* span[4];
    lb_joint* ctl = (t1 == lbt_control_joint) ? _joints[0] : (t2 == lbt_control_joint) ? _joints[1] : _joints[2];
    assert(ctl && (ctl->get_type() == lbt_control_joint));
    /* unfold span */
    auto* prev = ctl->get_prev_joint();
    auto* next = ctl->get_next_joint();
    assert(prev && next);
    if(prev->get_type() == lbt_control_joint) {
        span[0] = prev->get_prev_joint();
        span[1] = prev;
        span[2] = ctl;
        span[3] = next;
    }
    else if(next->get_type() == lbt_control_joint) {
        span[0] = prev;
        span[1] = ctl;
        span[2] = next;
        span[3] = next->get_next_joint();
    }
    else {
        span[0] = prev;
        span[1] = ctl;
        span[2] = next;
        span[3] = 0;
    }
    /* should hit all 3 */
    auto is_in_span = [&span](lb_joint* joint)-> bool {
        assert(joint);
        return (joint == span[0]) || (joint == span[1]) || (joint == span[2]) || (joint == span[3]);
    };
    bool all_in_span = is_in_span(_joints[0]) && is_in_span(_joints[1]) && is_in_span(_joints[2]);
    return all_in_span ? bf_klm_cr : bf_cr;
}

bool bat_triangle::is_overlapped(const bat_triangle& other) const
{
    const_cast<bat_triangle*>(this)->ensure_make_reduced();
    const_cast<bat_triangle*>(&other)->ensure_make_reduced();
    const vec2& p1 = get_reduced_point(0);
    const vec2& p2 = get_reduced_point(1);
    const vec2& p3 = get_reduced_point(2);
    const vec2& q1 = other.get_reduced_point(0);
    const vec2& q2 = other.get_reduced_point(1);
    const vec2& q3 = other.get_reduced_point(2);
    if(pink::point_in_triangle(p1, q1, q2, q3) ||
        pink::point_in_triangle(p2, q1, q2, q3) ||
        pink::point_in_triangle(p3, q1, q2, q3)
        )
        return true;
    return pink::point_in_triangle(q1, p1, p2, p3);
}

void bat_triangle::ensure_make_reduced()
{
    if(_is_reduced)
        return;
    assert(_joints[0] && _joints[1] && _joints[2]);
    _is_reduced = true;
    const vec2& p1 = get_point(0);
    const vec2& p2 = get_point(1);
    const vec2& p3 = get_point(2);
    vec2 v1, v2, v3, pebi1, pebi2, pebi3;
    v1.sub(p2, p1);
    v2.sub(p3, p2);
    v3.sub(p1, p3);
    v1.normalize();
    v2.normalize();
    v3.normalize();
    pebi1.sub(v1, v3);
    pebi2.sub(v2, v1);
    pebi3.sub(v3, v2);
    pebi1.normalize();
    pebi2.normalize();
    pebi3.normalize();
    const float d = 0.1f;
    pebi1.scale(d);
    pebi2.scale(d);
    pebi3.scale(d);
    _reduced[0].add(p1, pebi1);
    _reduced[1].add(p2, pebi2);
    _reduced[2].add(p3, pebi3);
}

void bat_triangle::tracing() const
{
    auto& p1 = get_point(0);
    auto& p2 = get_point(1);
    auto& p3 = get_point(2);
    trace(_t("@moveTo %f, %f;\n"), p1.x, p1.y);
    trace(_t("@lineTo %f, %f;\n"), p2.x, p2.y);
    trace(_t("@lineTo %f, %f;\n"), p3.x, p3.y);
    trace(_t("@lineTo %f, %f;\n"), p1.x, p1.y);
}

void bat_triangle::trace_reduced_points() const
{
    auto& p1 = get_reduced_point(0);
    auto& p2 = get_reduced_point(1);
    auto& p3 = get_reduced_point(2);
    trace(_t("@moveTo %f, %f;\n"), p1.x, p1.y);
    trace(_t("@lineTo %f, %f;\n"), p2.x, p2.y);
    trace(_t("@lineTo %f, %f;\n"), p3.x, p3.y);
    trace(_t("@lineTo %f, %f;\n"), p1.x, p1.y);
}

batch_processor::batch_processor()
{
}

batch_processor::~batch_processor()
{
    clear_batches();
}

void batch_processor::add_polygon(lb_polygon* poly)
{
    assert(poly);
    auto& cdt = poly->get_cdt_result();
    cdt.traverse_triangles([this](void* i, void* j, void* k) {
        assert(i && j && k);
        add_triangle(
            reinterpret_cast<lb_joint*>(i),
            reinterpret_cast<lb_joint*>(j),
            reinterpret_cast<lb_joint*>(k)
            );
        /*
        auto* a = reinterpret_cast<lb_joint*>(i);
        auto* b = reinterpret_cast<lb_joint*>(j);
        auto* c = reinterpret_cast<lb_joint*>(k);
        auto& p1 = a->get_point();
        auto& p2 = b->get_point();
        auto& p3 = c->get_point();
        trace(_t("@moveTo %f, %f;\n"), p1.x, p1.y);
        trace(_t("@lineTo %f, %f;\n"), p2.x, p2.y);
        trace(_t("@lineTo %f, %f;\n"), p3.x, p3.y);
        trace(_t("@lineTo %f, %f;\n"), p1.x, p1.y);
        */
    });
}

void batch_processor::clear_batches()
{
    for(auto* p : _batches) { gs_del(bat_batch, p); }
    for(auto* p : _triangles) { gs_del(bat_triangle, p); }
    _batches.clear();
    _triangles.clear();
}

bat_batch* batch_processor::create_batch(bat_type t)
{
    auto* p = gs_new(bat_batch, t);
    assert(p);
    _batches.push_back(p);
    return p;
}

bat_triangle* batch_processor::create_triangle(lb_joint* j1, lb_joint* j2, lb_joint* j3)
{
    assert(j1 && j2 && j3);
    auto* p = gs_new(bat_triangle, j1, j2, j3);
    _triangles.push_back(p);
    return p;
}

void batch_processor::add_triangle(lb_joint* j1, lb_joint* j2, lb_joint* j3)
{
    assert(j1 && j2 && j3);
    auto* triangle = create_triangle(j1, j2, j3);
    assert(triangle);
    rectf rc;
    triangle->make_rect(rc);
    auto t = triangle->decide();
    auto find_next_batch = [](bat_type t, bat_iter from, bat_iter to)->bat_iter {
        for(auto i = from; i != to; ++ i) {
            if((*i)->get_type() == t)
                return i;
        }
        return to;
    };
    auto f = find_next_batch(t, _batches.begin(), _batches.end());
    while(f != _batches.end()) {
        auto* bat = *f;
        assert(bat);
        auto& rtr = bat->get_rtree();
        vector<bat_triangle*> result;
        rtr.query(rc, result);
        bool overlapped = false;
        for(auto* p : result) {
            assert(p);
            if(p->is_overlapped(*triangle)) {
                overlapped = true;
                break;
            }
        }
        if(!overlapped) {
            rtr.insert(triangle, rc);
            return;
        }
        f = find_next_batch(t, ++ f, _batches.end());
    }
    assert(f == _batches.end());
    auto* bat = create_batch(t);
    assert(bat);
    auto& rtr = bat->get_rtree();
    rtr.insert(triangle, rc);
}

__ariel_end__
