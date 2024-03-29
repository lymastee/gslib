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

#include <ariel/batch.h>
#include <ariel/painter.h>
#include <gslib/utility.h>

__ariel_begin__

static bat_type bat_decide_type(uint brush_tag, bool has_klm)
{
    if(has_klm) {
        switch(brush_tag)
        {
        case painter_brush::solid:
            return bf_klm_cr;
        case painter_brush::picture:
            return bf_klm_tex;
        default:
            assert(!"unexpected tag.");
            return bf_klm_cr;
        }
    }
    else {
        switch(brush_tag)
        {
        case painter_brush::solid:
            return bf_cr;
        case painter_brush::picture:
            return bf_klm_tex;      /* use bf_klm_tex for better batching. */
        default:
            assert(!"unexpected tag.");
            return bf_cr;
        }
    }
}

static bool bat_is_triangle_overlapped(const bat_triangle* triangle, const rectf& rc, const bat_batch* batch)
{
    assert(triangle && batch);
    assert((batch->get_type() >= bf_start) && (batch->get_type() <= bf_end));
    auto& rtr = static_cast<const bat_fill_batch*>(batch)->const_rtree();
    vector<bat_triangle*> result;
    rtr.query(rc, result);
    for(auto* p : result) {
        assert(p);
        if(p->is_overlapped(*triangle))
            return true;
    }
    return false;
}

static bool bat_is_triangle_overlapped(const bat_triangle* triangle, const bat_batch* batch)
{
    assert(triangle && batch);
    rectf rc;
    triangle->make_rect(rc);
    return bat_is_triangle_overlapped(triangle, rc, batch);
}

static bool bat_is_triangles_overlapped(const bat_triangles& triangles, const bat_batch* batch)
{
    assert(batch);
    for(const bat_triangle* p : triangles) {
        assert(p);
        if(bat_is_triangle_overlapped(p, batch))
            return true;
    }
    return false;
}

bat_triangle::bat_triangle()
{
    _joints[0] = _joints[1] = _joints[2] = 0;
    _zorder = -1.f;
    _is_reduced = false;
}

bat_triangle::bat_triangle(lb_joint* i, lb_joint* j, lb_joint* k)
{
    _joints[0] = i;
    _joints[1] = j;
    _joints[2] = k;
    _zorder = -1.f;
    _is_reduced = false;
}

void bat_triangle::make_rect(rectf& rc) const
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

bat_type bat_triangle::decide(uint brush_tag) const
{
    return bat_decide_type(brush_tag, has_klm_coords());
}

bool bat_triangle::has_klm_coords() const
{
    assert(_joints[0] && _joints[1] && _joints[2]);
    auto t1 = _joints[0]->get_type();
    auto t2 = _joints[1]->get_type();
    auto t3 = _joints[2]->get_type();
    if((t1 == lbt_end_joint) && (t2 == lbt_end_joint) && (t3 == lbt_end_joint))
        return false;
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
    return all_in_span;
}

const vec2& bat_triangle::get_reduced_point(int i) const
{
    assert(_is_reduced);
    return _reduced[i];
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
    /* situation like p1 == q1 && p2 == q2 && p3 == q3 was definitely overlapped, but missed, so here we should also judge its center */
    vec2 c;
    get_center(c);
    if(point_in_triangle(p1, q1, q2, q3) ||
        point_in_triangle(p2, q1, q2, q3) ||
        point_in_triangle(p3, q1, q2, q3) ||
        point_in_triangle(c, q1, q2, q3)
        )
        return true;
    return point_in_triangle(q1, p1, p2, p3);
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

void bat_triangle::get_center(vec2& c) const
{
    c.add(get_point(0), get_point(1));
    c += get_point(2);
    c.scale(1.f / 3.f);
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
    _zorder = -1.f;
    _srcjs[0] = _srcjs[1] = 0;
    _tag = painter_pen::none;
    _half = false;
    _recalc = true;
}

void bat_line::setup_coef()
{
    get_linear_coefficient(_coef, _points[0], vec2().sub(_points[1], _points[0]));
    _coef.normalize();
}

float bat_line::get_line_length() const
{
    return vec2().sub(_points[1], _points[0]).length();
}

bat_type bat_line::decide() const
{
    switch(_tag)
    {
    case painter_pen::solid:
        return bs_coef_cr;
    case painter_pen::picture:
        return bs_coef_tex;
    default:
        assert(!"unexpected tag.");
        return bs_coef_cr;
    }
}

void bat_line::get_bound_rect(rectf& rc) const
{
    rc.left = min(_points[0].x, _points[1].x);
    rc.right = max(_points[0].x, _points[1].x);
    rc.top = min(_points[0].y, _points[1].y);
    rc.bottom = max(_points[0].y, _points[1].y);
}

static void trace_clip_triangle(const bat_triangle* triangle, bat_line output[2], int c)
{
#if (defined (DEBUG) || defined (_DEBUG)) && defined(_GS_DEBUG_VERBOSE)
    assert(triangle && output);
    if(c <= 0)
        return;
    trace(_t("#clip triangle:\n"));
    triangle->tracing();
    for(int i = 0; i < c; i ++)
        output[i].tracing();
#endif
}

int bat_line::clip_triangle(bat_line output[2], const bat_triangle* triangle) const
{
    assert(triangle);
    auto& p1 = triangle->get_point(0);
    auto& p2 = triangle->get_point(1);
    auto& p3 = triangle->get_point(2);
    const_cast<bat_triangle*>(triangle)->ensure_make_reduced();
    bool inside1 = point_in_triangle(_points[0], triangle->get_reduced_point(0), triangle->get_reduced_point(1), triangle->get_reduced_point(2));
    bool inside2 = point_in_triangle(_points[1], triangle->get_reduced_point(0), triangle->get_reduced_point(1), triangle->get_reduced_point(2));
    auto clone_line_info = [this](bat_line& line) {
        line.set_pen_tag(_tag);
        line.set_source_joint(0, get_source_joint(0));
        line.set_source_joint(1, get_source_joint(1));
        line.set_zorder(get_zorder());
        line.set_line_width(get_line_width());
        line.set_half_line(is_half_line());
        line.set_coef(get_coef());
    };
    auto test_intersection = [](vec2& p, const vec2& p1, const vec2& p2, const vec2& p3, const vec2& p4)-> bool {
        vec2 d1, d2;
        d1.sub(p2, p1);
        d2.sub(p4, p3);
        if(is_parallel(d1, d2))
            return false;
        intersectp_linear_linear(p, p1, p3, d1, d2);
        float t1 = linear_reparameterize(p1, p2, p);
        float t2 = linear_reparameterize(p3, p4, p);
        return (t1 >= 0.f) && (t1 <= 1.f) && (t2 >= 0.f) && (t2 <= 1.f);
    };
    if(inside1 && inside2)
        return 0;
    else if(inside1) {
        vec2 p;
        bool is_intersected = test_intersection(p, _points[0], _points[1], p1, p2) ||
            test_intersection(p, _points[0], _points[1], p2, p3) ||
            test_intersection(p, _points[0], _points[1], p3, p1);
        if(!is_intersected)
            p = _points[1];
        clone_line_info(output[0]);
        output[0].set_start_point(_points[0]);
        output[0].set_end_point(p);
        trace_clip_triangle(triangle, output, 1);
        return 1;
    }
    else if(inside2) {
        vec2 p;
        bool is_intersected = test_intersection(p, _points[0], _points[1], p1, p2) ||
            test_intersection(p, _points[0], _points[1], p2, p3) ||
            test_intersection(p, _points[0], _points[1], p3, p1);
        if(!is_intersected)
            p = _points[0];
        clone_line_info(output[0]);
        output[0].set_start_point(p);
        output[0].set_end_point(_points[1]);
        trace_clip_triangle(triangle, output, 1);
        return 1;
    }
    vector<vec2> intps;
    vec2 p;
    if(test_intersection(p, _points[0], _points[1], p1, p2))
        intps.push_back(p);
    if(test_intersection(p, _points[0], _points[1], p2, p3))
        intps.push_back(p);
    if(test_intersection(p, _points[0], _points[1], p3, p1))
        intps.push_back(p);
    assert(intps.size() <= 2);
    if((intps.size() == 2) && (vec2().sub(intps.front(), intps.back()).length() > 0.5f)) {
        clone_line_info(output[0]);
        clone_line_info(output[1]);
        const vec2 *c1 = &intps.front(), *c2 = &intps.back();
        float lsq1 = vec2().sub(_points[0], *c1).lengthsq();
        float lsq2 = vec2().sub(_points[0], *c2).lengthsq();
        if(lsq1 > lsq2)
            gs_swap(c1, c2);
        output[0].set_start_point(_points[0]);
        output[0].set_end_point(*c1);
        output[1].set_start_point(*c2);
        output[1].set_end_point(_points[1]);
        trace_clip_triangle(triangle, output, 2);
        return 2;
    }
    return -1;      /* itself */
}

void bat_line::calc_contour_points()
{
    auto offset_line = [](const vec2& p1, const vec2& p2, vec2& p3, vec2& p4, float dist) {
        vec2 d;
        d.sub(p2, p1);
        vec2 c(d.y, -d.x);
        c.normalize();
        c.scale(dist);
        p3.add(p1, c);
        p4.add(p2, c);
    };
    if(is_half_line()) {
        float w = get_line_width();
        offset_line(_points[0], _points[1], _contourpt[0], _contourpt[1], 0.1f * w);
        offset_line(_points[0], _points[1], _contourpt[2], _contourpt[3], -0.9f * w);
    }
    else {
        float w = get_line_width() * 0.5f;
        offset_line(_points[0], _points[1], _contourpt[0], _contourpt[1], w);
        offset_line(_points[0], _points[1], _contourpt[2], _contourpt[3], -w);
    }
    _recalc = false;
}

void bat_line::trim_contour(bat_line& line)
{
    assert(!_recalc);
    assert(get_end_point() == line.get_start_point());
    static const float con_limit = 2.f;     /* 60 degree */
    const vec2& p1 = get_start_point();
    const vec2& p2 = get_end_point();
    const vec2& p3 = line.get_end_point();
    vec2 d1, d2, bisector;
    d1.sub(p2, p1).normalize();
    d2.sub(p3, p2).normalize();
    bisector.sub(d1, d2);
    if(is_parallel(bisector, d1)) {
        set_tail_con(bct_normal);
        line.set_head_con(bct_normal);
        return;
    }
    vec2 up, down;
    intersectp_linear_linear(up, get_contour_point(1), p2, d1, bisector);
    intersectp_linear_linear(down, get_contour_point(3), p2, d1, bisector);
    float d = vec2().sub(up, p2).length();
    if(d <= con_limit) {
        set_contour_point(1, up);
        set_contour_point(3, down);
        line.set_contour_point(0, up);
        line.set_contour_point(2, down);
        set_tail_con(bct_normal);
        line.set_head_con(bct_normal);
        return;
    }
    assert(d > con_limit);
    bisector.normalize().scale(con_limit);
    bool is_convex = d1.ccw(d2) > 0.f;      /* -y direction */
    float mhd = gs_min(get_line_length(), line.get_line_length()) / 2.f;
    bool use_wedge = d < mhd;
    if(is_convex) {
        up = p2 + bisector;
        if(use_wedge) {
            set_contour_point(3, down);
            line.set_contour_point(2, down);
            set_tail_point(up);
            line.set_head_point(up);
            set_tail_con(bct_convex_wedge);
            line.set_head_con(bct_convex_wedge);
        }
        else {
            set_tail_point(up);
            line.set_head_point(up);
            set_tail_con(bct_convex_notch);
            line.set_head_con(bct_convex_notch);
        }
    }
    else {
        down = p2 + bisector;
        if(use_wedge) {
            set_contour_point(1, up);
            line.set_contour_point(0, up);
            set_tail_point(down);
            line.set_head_point(down);
            set_tail_con(bct_concave_wedge);
            line.set_head_con(bct_concave_wedge);
        }
        else {
            set_tail_point(down);
            line.set_head_point(down);
            set_tail_con(bct_concave_notch);
            line.set_head_con(bct_concave_notch);
        }
    }
}

void bat_line::tracing() const
{
    trace(_t("@moveTo %f, %f;\n"), _points[0].x, _points[0].y);
    trace(_t("@lineTo %f, %f;\n"), _points[1].x, _points[1].y);
    trace(_t("#tracing contour info.\n"));
    trace(_t("@moveTo %f, %f;\n"), _contourpt[0].x, _contourpt[0].y);
    trace(_t("@lineTo %f, %f;\n"), _contourpt[1].x, _contourpt[1].y);
    trace(_t("@lineTo %f, %f;\n"), _contourpt[3].x, _contourpt[3].y);
    trace(_t("@lineTo %f, %f;\n"), _contourpt[2].x, _contourpt[2].y);
    trace(_t("@lineTo %f, %f;\n"), _contourpt[0].x, _contourpt[0].y);
}

bat_line* bat_line::create_line(lb_joint* i, lb_joint* j, float w, float z, uint t, bool half)
{
    assert(i && j);
    auto* p = new bat_line;
    assert(p);
    p->set_source_joint(0, i);
    p->set_source_joint(1, j);
    p->set_zorder(z);
    p->set_pen_tag(t);
    p->set_half_line(half);
    p->set_line_width(w);
    p->set_start_point(i->get_point());
    p->set_end_point(j->get_point());
    p->setup_coef();
    return p;
}

bat_line* bat_line::create_half_line(lb_joint* i, lb_joint* j, const vec2& p1, const vec2& p2, float w, float z, uint t)
{
    assert(i && j);
    auto* p = new bat_line;
    assert(p);
    p->set_source_joint(0, i);
    p->set_source_joint(1, j);
    p->set_pen_tag(t);
    p->set_zorder(z);
    p->set_half_line(true);
    p->set_line_width(w);
    p->set_start_point(p1);
    p->set_end_point(p2);
    p->setup_coef();
    return p;
}

bat_stroke_host_batch::~bat_stroke_host_batch()
{
    for(auto* p : _lines)
        delete p;
    _lines.clear();
}

batch_processor::batch_processor()
{
    _antialias = false;
}

batch_processor::~batch_processor()
{
    clear_batches();
}

void batch_processor::add_non_tex_polygon(lb_polygon* poly, float z, uint brush_tag)
{
    assert(poly);
    assert(brush_tag != painter_brush::picture);
    auto& cdt = poly->get_cdt_result();
    dt_traversal_triangles dtts;
    cdt.collect_triangles(dtts);
    for(const dt_traversal_triangle& dtt : dtts) {
        auto* j1 = reinterpret_cast<lb_joint*>(dtt.binding1);
        auto* j2 = reinterpret_cast<lb_joint*>(dtt.binding2);
        auto* j3 = reinterpret_cast<lb_joint*>(dtt.binding3);
#if (defined (DEBUG) || defined (_DEBUG)) && defined(_GS_DEBUG_VERBOSE)
        auto& p1 = j1->get_point();
        auto& p2 = j2->get_point();
        auto& p3 = j3->get_point();
        trace(_t("@moveTo %f, %f;\n"), p1.x, p1.y);
        trace(_t("@lineTo %f, %f;\n"), p2.x, p2.y);
        trace(_t("@lineTo %f, %f;\n"), p3.x, p3.y);
        trace(_t("@lineTo %f, %f;\n"), p1.x, p1.y);
#endif
        add_triangle(j1, j2, j3, z, brush_tag);
    }
}

/* I put the picture fill into one batch for better texture batching purpose. */
bat_batch* batch_processor::add_tex_polygons(lb_polygon_list& polys, float z)
{
    if(polys.empty())
        return nullptr;
    /* gather all the tex triangles */
    bat_triangles triangles;
    for(auto* poly : polys) {
        assert(poly);
        gather_tex_triangles(triangles, poly, z);
    }
    if(triangles.empty())
        return nullptr;
    /* find which batch could contain the whole polygon */
    auto* f = find_containable_tex_batch(triangles);
    if(!f) {
        f = create_batch<bat_fill_batch>(bf_klm_tex);
        if(is_aa_enabled()) {
            /* for every bf_klm_tex batch, will have a bs_coef_tex batch for boundary anti-aliasing. */
            create_batch<bat_stroke_host_batch>(bs_coef_tex);
        }
    }
    assert(f && (f->get_type() == bf_klm_tex));
    /* insert all the triangles into the batch */
    auto& rtr = static_cast<bat_fill_batch*>(f)->get_rtree();
    for(auto* t : triangles) {
        assert(t);
        rectf rc;
        t->make_rect(rc);
        rtr.insert(t, rc);
    }
    return f;
}

bat_line* batch_processor::add_line(lb_joint* i, lb_joint* j, float w, float z, uint pen_tag)
{
    assert(i && j);
    return create_line(i, j, w, z, pen_tag, false);
}

bat_line* batch_processor::add_aa_border(lb_joint* i, lb_joint* j, float z, uint pen_tag)
{
    assert(i && j);
    static const float aa_width = 1.3f;
    return create_half_line(i, j, i->get_point(), j->get_point(), aa_width, z, pen_tag);
}

void batch_processor::finish_batching()
{
    proceed_line_batch();
}

void batch_processor::clear_batches()
{
    for(auto* p : _batches) { delete p; }
    for(auto* p : _triangles) { delete p; }
    for(auto* p : _lines) { delete p; }
    _batches.clear();
    _triangles.clear();
    _lines.clear();
}

template<class _batch>
_batch* batch_processor::create_batch(bat_type t)
{
    auto* p = new _batch(t);
    assert(p);
    _batches.push_back(p);
    return p;
}

bat_triangle* batch_processor::create_triangle(lb_joint* j1, lb_joint* j2, lb_joint* j3, float z)
{
    assert(j1 && j2 && j3);
    auto* p = new bat_triangle(j1, j2, j3);
    p->set_zorder(z);
    _triangles.push_back(p);
    return p;
}

bat_line* batch_processor::create_line(lb_joint* i, lb_joint* j, float w, float z, uint t, bool half)
{
    assert(i && j);
    auto* p = bat_line::create_line(i, j, w, z, t, half);
    assert(p);
    _lines.push_back(p);
    return p;
}

bat_line* batch_processor::create_half_line(lb_joint* i, lb_joint* j, const vec2& p1, const vec2& p2, float w, float z, uint t)
{
    assert(i && j);
    auto* p = bat_line::create_half_line(i, j, p1, p2, w, z, t);
    assert(p);
    _lines.push_back(p);
    return p;
}

void batch_processor::add_triangle(lb_joint* j1, lb_joint* j2, lb_joint* j3, float z, uint brush_tag)
{
    assert(j1 && j2 && j3);
    auto* triangle = create_triangle(j1, j2, j3, z);
    assert(triangle);
    // collect_aa_borders(triangle, b);
    rectf rc;
    triangle->make_rect(rc);
    auto t = triangle->decide(brush_tag);
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
        if(!bat_is_triangle_overlapped(triangle, rc, bat)) {
            auto& rtr = static_cast<bat_fill_batch*>(bat)->get_rtree();
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

void batch_processor::collect_aa_borders(bat_triangle* triangle, bool b[3], uint pen_tag)
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
        return !is_concave_angle(j1->get_point(), ja->get_point(), j2->get_point());
    };
    if(b[0] && is_valid_border(j1, j2, j3))
        add_aa_border(j1, j2, triangle->get_zorder(), pen_tag);
    if(b[1] && is_valid_border(j2, j3, j1))
        add_aa_border(j2, j3, triangle->get_zorder(), pen_tag);
    if(b[2] && is_valid_border(j3, j1, j2))
        add_aa_border(j3, j1, triangle->get_zorder(), pen_tag);
}

static void bat_clip_triangles(bat_lines& out_lines, bat_lines& line_holdings, const bat_triangles& triangles, bat_triangles::const_iterator i, bat_line* line, bat_line* orgline)
{
    assert(line && orgline);
    assert(i != triangles.end());
    auto record_line = [&out_lines, &line_holdings, &orgline](bat_line* line) {
        assert(line);
        if(line == orgline)
            out_lines.push_back(line);
        else {
            auto* p = new bat_line(*line);
            out_lines.push_back(p);
            line_holdings.push_back(p);
        }
    };
    bat_line output[2];
    int c = line->clip_triangle(output, *i);
    if(++ i == triangles.end()) {
        if(c == -1)
            record_line(line);
        else if(c == 1)
            record_line(&output[0]);
        else if(c == 2) {
            record_line(&output[0]);
            record_line(&output[1]);
        }
        return;
    }
    if(c == -1)
        bat_clip_triangles(out_lines, line_holdings, triangles, i, line, orgline);
    else if(c == 1)
        bat_clip_triangles(out_lines, line_holdings, triangles, i, &output[0], orgline);
    else if(c == 2) {
        bat_clip_triangles(out_lines, line_holdings, triangles, i, &output[0], orgline);
        bat_clip_triangles(out_lines, line_holdings, triangles, i, &output[1], orgline);
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
            if((batch->get_type() >= bf_start) && (batch->get_type() <= bf_end)) {
                bat_triangles ovltris;
                auto& rtr = static_cast<bat_fill_batch*>(batch)->get_rtree();
                rtr.query(rc, ovltris);
                for(auto* p : ovltris) {
                    assert(p);
                    if(z < p->get_zorder())
                        triangles.push_back(p);
                }
            }
        }
    };
    bat_lines out_lines, line_holdings;
    for(auto* line : _lines) {
        assert(line);
        bat_triangles ovltris;
        query_triangles(ovltris, line);
        if(ovltris.empty()) {
            out_lines.push_back(line);
            continue;
        }
        bat_clip_triangles(out_lines, line_holdings, ovltris, ovltris.begin(), line, line);
    }
    if(out_lines.empty())
        return;
    if(!line_holdings.empty()) {
        _lines.insert(_lines.end(), line_holdings.begin(), line_holdings.end());
        line_holdings.clear();
    }
    /* generate batches */
    bat_stroke_batch *texbat = 0, *crbat = 0;
    for(auto* line : out_lines) {
        assert(line);
        auto t = line->decide();
        if(t == bs_coef_cr) {
            if(!crbat)
                crbat = new bat_stroke_batch(bs_coef_cr);
            crbat->get_lines().push_back(line);
        }
        else if(t == bs_coef_tex) {
            if(!texbat)
                texbat = new bat_stroke_batch(bs_coef_tex);
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

void batch_processor::gather_tex_triangles(bat_triangles& triangles, lb_polygon* poly, float z)
{
    assert(poly);
    auto& cdt = poly->get_cdt_result();
    dt_traversal_triangles dtts;
    cdt.collect_triangles(dtts);
    for(const dt_traversal_triangle& dtt : dtts) {
        auto* j1 = reinterpret_cast<lb_joint*>(dtt.binding1);
        auto* j2 = reinterpret_cast<lb_joint*>(dtt.binding2);
        auto* j3 = reinterpret_cast<lb_joint*>(dtt.binding3);
        auto* triangle = create_triangle(j1, j2, j3, z);
        assert(triangle);
        triangles.push_back(triangle);
    }
}

bat_batch* batch_processor::find_containable_tex_batch(const bat_triangles& triangles)
{
    auto find_next_batch = [&triangles](bat_reversed_iter from, bat_reversed_iter to)-> bat_reversed_iter {
        for(auto i = from; i != to; ++ i) {
            auto t = (*i)->get_type();
            if(t >= bs_start && t <= bs_end) {
                if(t == bs_coef_tex) {
                    auto n = std::next(i);
                    if(n != to && ((*n)->get_type() == bf_klm_tex))
                        continue;
                }
                return to;
            }
            assert(t >= bf_start && t <= bf_end);
            if(t == bf_klm_tex)
                return i;
            else {
                if(bat_is_triangles_overlapped(triangles, *i))
                    return to;
            }
        }
        return to;
    };
    bat_batch* lastfound = nullptr;
    auto f = find_next_batch(_batches.rbegin(), _batches.rend());
    while(f != _batches.rend()) {
        auto* bat = *f;
        assert(bat);
        assert(bat->get_type() == bf_klm_tex);
        if(bat_is_triangles_overlapped(triangles, bat))
            break;
        lastfound = bat;
        f = find_next_batch(++ f, _batches.rend());
    }
    return lastfound;
}

bat_stroke_batch* batch_processor::find_associated_tex_stroke_batch(const bat_batch* bat)
{
    assert(is_aa_enabled());
    assert(bat && (bat->get_type() == bf_klm_tex));
    auto f = _batches.begin();
    for(; f != _batches.end(); ++ f) {
        if(*f == bat)
            break;
    }
    if(f == _batches.end()) {
        assert(!"batch processor locate bat failed.");
        return nullptr;
    }
    if(++ f == _batches.end())
        return nullptr;
    auto* stroke_bat = *f;
    assert(stroke_bat);
    if(stroke_bat->get_type() != bs_coef_tex)
        return nullptr;
    return static_cast<bat_stroke_batch*>(stroke_bat);
}

__ariel_end__
