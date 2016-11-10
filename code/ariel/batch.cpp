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

#include <ariel/batch.h>
#include <pink/utility.h>

__ariel_begin__

bat_triangle::bat_triangle()
{
    _joints[0] = _joints[1] = _joints[2] = 0;
    _zorder = -1;
    _is_reduced = false;
}

bat_triangle::bat_triangle(lb_joint* i, lb_joint* j, lb_joint* k)
{
    _joints[0] = i;
    _joints[1] = j;
    _joints[2] = k;
    _zorder = -1;
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

bat_line::bat_line()
{
    _width = 2.f;
    _zorder = -1;
    _srcjs[0] = _srcjs[1] = 0;
    _half = false;
}

void bat_line::setup_coef()
{
    pink::get_linear_coefficient(_coef, _points[0], vec2().sub(_points[1], _points[0]));
    _coef.normalize();
}

bat_type bat_line::decide() const
{
    return bs_coef_cr;
}

void bat_line::get_bound_rect(rectf& rc) const
{
    rc.left = min(_points[0].x, _points[1].x);
    rc.right = max(_points[0].x, _points[1].x);
    rc.top = min(_points[0].y, _points[1].y);
    rc.bottom = max(_points[0].y, _points[1].y);
}

int bat_line::clip_triangle(bat_line output[2], const bat_triangle* triangle) const
{
    assert(triangle);
    auto& p1 = triangle->get_point(0);
    auto& p2 = triangle->get_point(1);
    auto& p3 = triangle->get_point(2);
    bool inside1 = pink::point_in_triangle(_points[0], p1, p2, p3);
    bool inside2 = pink::point_in_triangle(_points[1], p1, p2, p3);
    if(inside1 && inside2)
        return 0;
    else if(inside1) {

        return 1;
    }
    else if(inside2) {
        return 1;
    }
    return 0;
}

void bat_line::tracing() const
{
    trace(_t("@moveTo %f, %f;\n"), _points[0].x, _points[0].y);
    trace(_t("@lineTo %f, %f;\n"), _points[1].x, _points[1].y);
}

batch_processor::batch_processor()
{
}

batch_processor::~batch_processor()
{
    clear_batches();
}

void batch_processor::add_polygon(lb_polygon* poly, int z)
{
    assert(poly);
    auto& cdt = poly->get_cdt_result();
    cdt.traverse_triangles([this, &z](void* i, void* j, void* k, bool b[3]) {
        assert(i && j && k);
        auto* j1 = reinterpret_cast<lb_joint*>(i);
        auto* j2 = reinterpret_cast<lb_joint*>(j);
        auto* j3 = reinterpret_cast<lb_joint*>(k);
#ifdef _DEBUG
        auto& p1 = j1->get_point();
        auto& p2 = j2->get_point();
        auto& p3 = j3->get_point();
        trace(_t("@moveTo %f, %f;\n"), p1.x, p1.y);
        trace(_t("@lineTo %f, %f;\n"), p2.x, p2.y);
        trace(_t("@lineTo %f, %f;\n"), p3.x, p3.y);
        trace(_t("@lineTo %f, %f;\n"), p1.x, p1.y);
#endif
        add_triangle(j1, j2, j3, b, z);
    });
}

void batch_processor::add_line(lb_joint* i, lb_joint* j, float w, int z)
{
    assert(i && j);
    create_line(i, j, w, z);
}

void batch_processor::add_aa_border(lb_joint* i, lb_joint* j, int z)
{
    /* offset a bit */
    assert(i && j);
    static const float aa_width = 2.f;
//     auto offset_line = [](const vec2& p1, const vec2& p2, vec2& p3, vec2& p4, float dist) {
//         vec2 d;
//         d.sub(p2, p1);
//         vec2 xd(d.y, -d.x);
//         xd.normalize();
//         xd.scale(dist);
//         p3.add(p1, xd);
//         p4.add(p2, xd);
//     };
//     auto& p1 = i->get_point();
//     auto& p2 = j->get_point();
//     vec2 s1, s2;
//     offset_line(p1, p2, s1, s2, 5.f);
// 
//     trace(_t("@moveTo %f, %f;\n"), s1.x, s1.y);
//     trace(_t("@lineTo %f, %f;\n"), s2.x, s2.y);

    create_half_line(i, j, i->get_point(), j->get_point(), aa_width, z);
}

void batch_processor::finish_batching()
{
    proceed_line_batch();
}

void batch_processor::clear_batches()
{
    for(auto* p : _batches) { gs_del(bat_batch, p); }
    for(auto* p : _triangles) { gs_del(bat_triangle, p); }
    for(auto* p : _lines) { gs_del(bat_line, p); }
    _batches.clear();
    _triangles.clear();
    _lines.clear();
}

template<class _batch>
_batch* batch_processor::create_batch(bat_type t)
{
    auto* p = gs_new(_batch, t);
    assert(p);
    _batches.push_back(p);
    return p;
}

bat_triangle* batch_processor::create_triangle(lb_joint* j1, lb_joint* j2, lb_joint* j3, int z)
{
    assert(j1 && j2 && j3);
    auto* p = gs_new(bat_triangle, j1, j2, j3);
    p->set_zorder(z);
    _triangles.push_back(p);
    return p;
}

bat_line* batch_processor::create_line(lb_joint* i, lb_joint* j, float w, int z)
{
    assert(i && j);
    auto* p = gs_new(bat_line);
    assert(p);
    p->set_source_joint(0, i);
    p->set_source_joint(1, j);
    p->set_zorder(z);
    p->set_half_line(false);
    p->set_line_width(w);
    p->set_start_point(i->get_point());
    p->set_end_point(j->get_point());
    p->setup_coef();
    _lines.push_back(p);
    return p;
}

bat_line* batch_processor::create_half_line(lb_joint* i, lb_joint* j, const vec2& p1, const vec2& p2, float w, int z)
{
    assert(i && j);
    auto* p = gs_new(bat_line);
    assert(p);
    p->set_source_joint(0, i);
    p->set_source_joint(1, j);
    p->set_zorder(z);
    p->set_half_line(true);
    p->set_line_width(w);
    p->set_start_point(p1);
    p->set_end_point(p2);
    p->setup_coef();
    _lines.push_back(p);
    return p;
}

void batch_processor::add_triangle(lb_joint* j1, lb_joint* j2, lb_joint* j3, bool b[3], int z)
{
    assert(j1 && j2 && j3);
    auto* triangle = create_triangle(j1, j2, j3, z);
    assert(triangle);
    collect_aa_borders(triangle, b);
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
        assert((bat->get_type() >= bf_start) && (bat->get_type() <= bf_end));
        auto& rtr = static_cast<bat_fill_batch*>(bat)->get_rtree();
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
    auto* bat = create_batch<bat_fill_batch>(t);
    assert(bat);
    assert((bat->get_type() >= bf_start) && (bat->get_type() <= bf_end));
    auto& rtr = bat->get_rtree();
    rtr.insert(triangle, rc);
}

void batch_processor::collect_aa_borders(bat_triangle* triangle, bool b[3])
{
    assert(triangle);
    auto* j1 = triangle->get_joint(0);
    auto* j2 = triangle->get_joint(1);
    auto* j3 = triangle->get_joint(2);
    assert(j1 && j2 && j3);
    auto is_valid_border = [](const lb_joint* j1, const lb_joint* j2, const lb_joint* ja)-> bool {
        assert(j1 && j2 && ja);
        if(!j1->is_adjacent_joint(j2))
            return false;
        if((j1->get_type() == lbt_control_joint) || (j2->get_type() == lbt_control_joint))
            return false;
        if((ja->get_type() != lbt_control_joint) || j1->is_adjacent_joint(ja) || j2->is_adjacent_joint(ja))
            return true;
        return !pink::is_concave_angle(j1->get_point(), ja->get_point(), j2->get_point());
    };
    if(b[0] && is_valid_border(j1, j2, j3))
        add_aa_border(j1, j2, triangle->get_zorder());
    if(b[1] && is_valid_border(j2, j3, j1))
        add_aa_border(j2, j3, triangle->get_zorder());
    if(b[2] && is_valid_border(j3, j1, j2))
        add_aa_border(j3, j1, triangle->get_zorder());
}

static void bat_clip_triangles(bat_lines& line_holdings, const bat_triangles& triangles, bat_triangles::const_iterator i, bat_line* line)
{
    assert(line);
    assert(i != triangles.end());
    bat_line output[2];
    int c = line->clip_triangle(output, *i);
    if(++ i == triangles.end()) {
        if(c == 1)
            line_holdings.push_back(gs_new(bat_line, output[0]));
        else if(c == 2) {
            line_holdings.push_back(gs_new(bat_line, output[0]));
            line_holdings.push_back(gs_new(bat_line, output[1]));
        }
        return;
    }
    if(c == 1)
        bat_clip_triangles(line_holdings, triangles, i, &output[0]);
    else if(c == 2) {
        bat_clip_triangles(line_holdings, triangles, i, &output[0]);
        bat_clip_triangles(line_holdings, triangles, i, &output[1]);
    }
}

void batch_processor::proceed_line_batch()
{
    auto query_triangles = [this](bat_triangles& triangles, bat_line* line) {
        assert(line);
        rectf rc;
        line->get_bound_rect(rc);
        auto z = line->get_zorder();
        for(auto* batch : _batches) {
            assert(batch);
            assert((batch->get_type() >= bf_start) && (batch->get_type() <= bf_end));
            bat_triangles ovltris;
            auto& rtr = static_cast<bat_fill_batch*>(batch)->get_rtree();
            rtr.query(rc, ovltris);
            for(auto* p : ovltris) {
                assert(p);
                if(z < p->get_zorder())
                    triangles.push_back(p);
            }
        }
    };
    bat_lines line_holdings;
    for(auto* line : _lines) {
        assert(line);
        bat_triangles ovltris;
        query_triangles(ovltris, line);
        if(ovltris.empty())
            continue;
        bat_clip_triangles(line_holdings, ovltris, ovltris.begin(), line);
    }
    if(line_holdings.empty())
        return;
    _lines.insert(_lines.end(), line_holdings.begin(), line_holdings.end());
    /* generate batches */
    bat_stroke_batch *texbat = 0, *crbat = 0;
    for(auto* line : line_holdings) {
        assert(line);
        auto t = line->decide();
        if(t == bs_coef_cr) {
            if(!crbat)
                crbat = gs_new(bat_stroke_batch, bs_coef_cr);
            crbat->get_lines().push_back(line);
        }
        else if(t == bs_coef_tex) {
            if(!texbat)
                texbat = gs_new(bat_stroke_batch, bs_coef_tex);
            texbat->get_lines().push_back(line);
        }
        else {
            assert(!"unexpected.");
        }
    }
    if(texbat)
        _batches.push_back(texbat);
    if(crbat)
        _batches.push_back(crbat);
}

__ariel_end__
