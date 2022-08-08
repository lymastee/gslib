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
#include <ariel/rose.h>
#include <ariel/textureop.h>

#if use_rendersys_d3d_11
#include <ariel/rosed3d11.cpp>
#endif

__ariel_begin__

template<class _vf>
static void rose_filling_klm_coords(bat_triangle* triangle, _vf pt[3])
{
    assert(triangle);
    auto* joint1 = triangle->get_joint(0);
    auto* joint2 = triangle->get_joint(1);
    auto* joint3 = triangle->get_joint(2);
    assert(joint1 && joint2 && joint3);
    auto t1 = joint1->get_type();
    auto t2 = joint2->get_type();
    auto t3 = joint3->get_type();
    if(t1 == lbt_end_joint) {
        auto* j1 = static_cast<lb_end_joint*>(joint1);
        if(t2 == lbt_end_joint) {
            auto* j2 = static_cast<lb_end_joint*>(joint2);
            assert(t3 == lbt_control_joint);
            auto* j3 = static_cast<lb_control_joint*>(joint3);
            if((j1->get_prev_joint() == j3) || (j2->get_next_joint() == j3)) {
                /* j2(ep) - j3(cp) - j1(ep) */
                pt[0].klm = j1->get_klm(0);
                pt[1].klm = j2->get_klm(1);
                pt[2].klm = j3->get_klm();
            }
            else {
                assert((j1->get_next_joint() == j3) || (j2->get_prev_joint() == j3));
                /* j1(ep) - j3(cp) - j2(ep) */
                pt[0].klm = j1->get_klm(1);
                pt[1].klm = j2->get_klm(0);
                pt[2].klm = j3->get_klm();
            }
        }
        else {
            assert(t2 == lbt_control_joint);
            auto* j2 = static_cast<lb_control_joint*>(joint2);
            if(t3 == lbt_end_joint) {
                auto* j3 = static_cast<lb_end_joint*>(joint3);
                if((j1->get_prev_joint() == j2) || (j3->get_next_joint() == j2)) {
                    /* j3(ep) - j2(cp) - j1(ep) */
                    pt[0].klm = j1->get_klm(0);
                    pt[1].klm = j2->get_klm();
                    pt[2].klm = j3->get_klm(1);
                }
                else {
                    assert((j1->get_next_joint() == j2) || (j3->get_prev_joint() == j2));
                    /* j1(ep) - j2(cp) - j3(ep) */
                    pt[0].klm = j1->get_klm(1);
                    pt[1].klm = j2->get_klm();
                    pt[2].klm = j3->get_klm(0);
                }
            }
            else {
                assert(t3 == lbt_control_joint);
                auto* j3 = static_cast<lb_control_joint*>(joint3);
                if((j1->get_prev_joint() == j2) || (j1->get_prev_joint() == j3)) {
                    /* (?) - (?) - j1 */
                    pt[0].klm = j1->get_klm(0);
                    pt[1].klm = j2->get_klm();
                    pt[2].klm = j3->get_klm();
                }
                else if((j1->get_next_joint() == j2) || (j1->get_next_joint() == j3)) {
                    /* j1 - (?) - (?) */
                    pt[0].klm = j1->get_klm(1);
                    pt[1].klm = j2->get_klm();
                    pt[2].klm = j3->get_klm();
                }
                else {
                    assert(!"unexpected.");
                    pt[0].klm = vec3(0.f, 0.f, 0.f);
                    pt[1].klm = vec3(0.f, 0.f, 0.f);
                    pt[2].klm = vec3(0.f, 0.f, 0.f);
                }
            }
        }
    }
    else {
        assert(t1 == lbt_control_joint);
        auto* j1 = static_cast<lb_control_joint*>(joint1);
        if(t2 == lbt_end_joint) {
            auto* j2 = static_cast<lb_end_joint*>(joint2);
            if(t3 == lbt_end_joint) {
                auto* j3 = static_cast<lb_end_joint*>(joint3);
                if((j2->get_prev_joint() == j1) || (j3->get_next_joint() == j1)) {
                    /* j3(ep) - j1(cp) - j2(ep) */
                    pt[0].klm = j1->get_klm();
                    pt[1].klm = j2->get_klm(0);
                    pt[2].klm = j3->get_klm(1);
                }
                else {
                    assert((j2->get_next_joint() == j1) || (j3->get_prev_joint() == j1));
                    /* j2(ep) - j1(cp) - j3(ep) */
                    pt[0].klm = j1->get_klm();
                    pt[1].klm = j2->get_klm(1);
                    pt[2].klm = j3->get_klm(0);
                }
            }
            else {
                assert(t3 == lbt_control_joint);
                auto* j3 = static_cast<lb_control_joint*>(joint3);
                if((j2->get_prev_joint() == j1) || (j2->get_prev_joint() == j3)) {
                    /* (?) - (?) - j2 */
                    pt[0].klm = j1->get_klm();
                    pt[1].klm = j2->get_klm(0);
                    pt[2].klm = j3->get_klm();
                }
                else if((j2->get_next_joint() == j1) || (j2->get_next_joint() == j3)) {
                    /* j2 - (?) - (?) */
                    pt[0].klm = j1->get_klm();
                    pt[1].klm = j2->get_klm(1);
                    pt[2].klm = j3->get_klm();
                }
                else {
                    assert(!"unexpected.");
                    pt[0].klm = vec3(0.f, 0.f, 0.f);
                    pt[1].klm = vec3(0.f, 0.f, 0.f);
                    pt[2].klm = vec3(0.f, 0.f, 0.f);
                }
            }
        }
        else {
            assert(t2 == lbt_control_joint);
            auto* j2 = static_cast<lb_control_joint*>(joint2);
            assert(t3 == lbt_end_joint);
            auto* j3 = static_cast<lb_end_joint*>(joint3);
            if((j3->get_prev_joint() == j1) || (j3->get_prev_joint() == j2)) {
                /* (?) - (?) - j3 */
                pt[0].klm = j1->get_klm();
                pt[1].klm = j2->get_klm();
                pt[2].klm = j3->get_klm(0);
            }
            else if((j3->get_next_joint() == j1) || (j3->get_next_joint() == j2)) {
                /* j3 - (?) - (?) */
                pt[0].klm = j1->get_klm();
                pt[1].klm = j2->get_klm();
                pt[2].klm = j3->get_klm(1);
            }
            else {
                assert(!"unexpected.");
                pt[0].klm = vec3(0.f, 0.f, 0.f);
                pt[1].klm = vec3(0.f, 0.f, 0.f);
                pt[2].klm = vec3(0.f, 0.f, 0.f);
            }
        }
    }
}

static bool is_image_transposed(texture2d* img, const rectf& rc)
{
    assert(img);
    int w, h;
    textureop::get_texture_dimension(img, w, h);
    assert((w == rc.width() && h == rc.height()) || (w == rc.height() && h == rc.width()));
    return !(w == rc.width() && h == rc.height());
}

static vec2 rose_calc_tex_coords(texture2d* img, const vec2& p, const tex_batcher& batch)
{
    assert(img);
    const auto& lm = batch.get_location_map();
    auto f = lm.find(img);
    assert(f != lm.end());
    mat3 mt, ms, m;
    mt.translation(f->second.left, f->second.top);
    ms.scaling(1.f / batch.get_width(), 1.f / batch.get_height());
    m.multiply(mt, ms);
    vec3 pt;
    is_image_transposed(img, f->second) ? pt.multiply(vec3(p.y, p.x, 1.f), m) :
        pt.multiply(vec3(p.x, p.y, 1.f), m);
    return vec2(pt.x / pt.z, pt.y / pt.z);
}

template<class _vf>
static void rose_filling_tex_coords(lb_joint* p, const tex_batcher& batch, _vf& pt)
{
    assert(p);
    auto* binding = static_cast<rose_bind_info_tex*>(p->get_binding());
    assert(binding);
    pt.tex = rose_calc_tex_coords(binding->img, binding->tex, batch);
}

rose_batch::rose_batch(int index)
{
    _bat_index = index;
    _vertex_shader = nullptr;
    _pixel_shader = nullptr;
    _vertex_format = nullptr;
    _vertex_buffer = nullptr;
}

rose_batch::~rose_batch()
{
    if(_vertex_buffer) {
        release_vertex_buffer(_vertex_buffer);
        _vertex_buffer = nullptr;
    }
}

void rose_batch::setup_vs_and_ps(rendersys* rsys)
{
    assert(rsys);
    assert(_vertex_shader && _pixel_shader);
    rsys->set_vertex_shader(_vertex_shader);
    rsys->set_pixel_shader(_pixel_shader);
}

void rose_batch::setup_vf_and_topology(rendersys* rsys, uint topo)
{
    assert(rsys);
    assert(_vertex_format);
    rsys->set_vertex_format(_vertex_format);
    rsys->set_render_option(opt_primitive_topology, topo);
}

void rose_fill_batch_cr::create(bat_batch* bat)
{
    assert(bat);
    switch(bat->get_type())
    {
    case bf_cr:
        return create_from_fill(static_cast<bat_fill_batch*>(bat));
    case bs_coef_cr:
        return create_from_stroke(static_cast<bat_stroke_batch*>(bat));
    default:
        assert(!"unexpected.");
        break;
    }
}

void rose_fill_batch_cr::tracing() const
{
    trace(_t("#start tracing fill batch cr:\n"));
    trace(_t("@!\n"));
    int i = 0, cap = (int)_vertices.size();
    for(; i != cap; i += 3) {
        auto& p1 = _vertices.at(i);
        auto& p2 = _vertices.at(i + 1);
        auto& p3 = _vertices.at(i + 2);
        trace(_t("@moveTo %f, %f;\n"), p1.pos.x, p1.pos.y);
        trace(_t("@lineTo %f, %f;\n"), p2.pos.x, p2.pos.y);
        trace(_t("@lineTo %f, %f;\n"), p3.pos.x, p3.pos.y);
        trace(_t("@lineTo %f, %f;\n"), p1.pos.x, p1.pos.y);
    }
    trace(_t("@@\n"));
}

void rose_fill_batch_cr::create_from_fill(bat_fill_batch* bat)
{
    assert(bat);
    auto& rtr = bat->get_rtree();
    rtr.for_each([this](bat_rtree_entity* ent) {
        assert(ent);
        if(!ent->get_bind_arg())
            return;
        auto* triangle = reinterpret_cast<bat_triangle*>(ent->get_bind_arg());
        assert(triangle);
        vertex_info_cr pt[3] =
        {
            { triangle->get_point(0), reinterpret_cast<rose_bind_info_cr*>(triangle->get_lb_binding(0))->color },
            { triangle->get_point(1), reinterpret_cast<rose_bind_info_cr*>(triangle->get_lb_binding(1))->color },
            { triangle->get_point(2), reinterpret_cast<rose_bind_info_cr*>(triangle->get_lb_binding(2))->color }
        };
        _vertices.push_back(pt[0]);
        _vertices.push_back(pt[1]);
        _vertices.push_back(pt[2]);
    });
}

void rose_fill_batch_cr::create_from_stroke(bat_stroke_batch* bat)
{
    assert(bat);
    auto& lines = bat->get_lines();
    for(auto* line : lines) {
        assert(line);
        /* point */
        auto& p1 = line->get_start_point();
        auto& p2 = line->get_end_point();
        if(line->need_recalc())
            line->calc_contour_points();
        auto* p = &line->get_contour_point(0);
        /* color */
        auto* sj1 = line->get_source_joint(0);
        auto* sj2 = line->get_source_joint(1);
        assert(sj1 && sj2);
        auto* os1 = reinterpret_cast<rose_bind_info_cr*>(sj1->get_binding());
        auto* os2 = reinterpret_cast<rose_bind_info_cr*>(sj2->get_binding());
        assert(os1 && os2);
        float t1 = gs_clamp(linear_reparameterize(sj1->get_point(), sj2->get_point(), p1), 0.f, 1.f);
        float t2 = gs_clamp(linear_reparameterize(sj1->get_point(), sj2->get_point(), p2), 0.f, 1.f);
        vec4 cr1, cr2;
        cr1.lerp(os1->color, os2->color, t1);
        cr2.lerp(os1->color, os2->color, t2);
        vertex_info_cr pt[] =
        {
            { p[0], cr1 },
            { p[1], cr2 },
            { p[2], cr1 },
            { p[3], cr2 },
        };
        _vertices.push_back(pt[0]);
        _vertices.push_back(pt[1]);
        _vertices.push_back(pt[2]);
        _vertices.push_back(pt[2]);
        _vertices.push_back(pt[1]);
        _vertices.push_back(pt[3]);
    }
}

void rose_fill_batch_klm_cr::create(bat_batch* bat)
{
    assert(bat);
    auto& rtr = static_cast<bat_fill_batch*>(bat)->get_rtree();
    rtr.for_each([this](bat_rtree_entity* ent) {
        assert(ent);
        if(!ent->get_bind_arg())
            return;
        auto* triangle = reinterpret_cast<bat_triangle*>(ent->get_bind_arg());
        assert(triangle);
        auto* joint1 = triangle->get_joint(0);
        auto* joint2 = triangle->get_joint(1);
        auto* joint3 = triangle->get_joint(2);
        vertex_info_klm_cr pt[3] =
        {
            { joint1->get_point(), vec3(), reinterpret_cast<rose_bind_info_cr*>(joint1->get_binding())->color },
            { joint2->get_point(), vec3(), reinterpret_cast<rose_bind_info_cr*>(joint2->get_binding())->color },
            { joint3->get_point(), vec3(), reinterpret_cast<rose_bind_info_cr*>(joint3->get_binding())->color }
        };
        rose_filling_klm_coords(triangle, pt);
        _vertices.push_back(pt[0]);
        _vertices.push_back(pt[1]);
        _vertices.push_back(pt[2]);
    });
}

void rose_fill_batch_klm_cr::tracing() const
{
    trace(_t("#start tracing fill batch klm cr:\n"));
    trace(_t("@!\n"));
    int i = 0, cap = (int)_vertices.size();
    for(; i != cap; i += 3) {
        auto& p1 = _vertices.at(i);
        auto& p2 = _vertices.at(i + 1);
        auto& p3 = _vertices.at(i + 2);
        trace(_t("@moveTo %f, %f;\n"), p1.pos.x, p1.pos.y);
        trace(_t("@lineTo %f, %f;\n"), p2.pos.x, p2.pos.y);
        trace(_t("@lineTo %f, %f;\n"), p3.pos.x, p3.pos.y);
        trace(_t("@lineTo %f, %f;\n"), p1.pos.x, p1.pos.y);
        trace(_t("#klm coords: %f, %f, %f, %f, %f, %f, %f, %f, %f;\n"),
            p1.klm.x, p1.klm.y, p1.klm.z,
            p2.klm.x, p2.klm.y, p2.klm.z,
            p3.klm.x, p3.klm.y, p3.klm.z
            );
    }
    trace(_t("@@\n"));
}

rose_fill_batch_klm_tex::rose_fill_batch_klm_tex(int index, render_sampler_state* ss):
    rose_batch(index)
{
    _sstate = ss;
    _srv = nullptr;
    _tex = nullptr;
}

void rose_fill_batch_klm_tex::create(bat_batch* bat)
{
    assert(bat);
    switch(bat->get_type())
    {
    case bf_klm_tex:
        return create_from_fill(static_cast<bat_fill_batch*>(bat));
    case bs_coef_tex:
        return create_from_stroke(static_cast<bat_stroke_batch*>(bat));
    default:
        assert(!"unexpected.");
        break;
    }
}

void rose_fill_batch_klm_tex::tracing() const
{
    trace(_t("#start tracing fill batch klm tex:\n"));
}

void rose_fill_batch_klm_tex::create_from_fill(bat_fill_batch* bat)
{
    assert(bat);
    auto& rtr = bat->get_rtree();
    /* deal with texture batch */
    assert(_texbatch.is_empty());
    rtr.for_each([this](bat_rtree_entity* ent) {
        assert(ent);
        if(!ent->get_bind_arg())
            return;
        auto* triangle = reinterpret_cast<bat_triangle*>(ent->get_bind_arg());
        assert(triangle);
        auto* joint1 = triangle->get_joint(0);
        auto* joint2 = triangle->get_joint(1);
        auto* joint3 = triangle->get_joint(2);
        assert(joint1 && joint2 && joint3);
        auto* bind1 = reinterpret_cast<rose_bind_info_tex*>(joint1->get_binding());
        auto* bind2 = reinterpret_cast<rose_bind_info_tex*>(joint2->get_binding());
        auto* bind3 = reinterpret_cast<rose_bind_info_tex*>(joint3->get_binding());
        assert(bind1 && bind2 && bind3);
        _texbatch.add_texture(bind1->img);
        _texbatch.add_texture(bind2->img);
        _texbatch.add_texture(bind3->img);
    });
    /* arrange texture batch */
    _texbatch.arrange();
    /* create vertices */
    rtr.for_each([this](bat_rtree_entity* ent) {
        assert(ent);
        if(!ent->get_bind_arg())
            return;
        auto* triangle = reinterpret_cast<bat_triangle*>(ent->get_bind_arg());
        assert(triangle);
        auto* joint1 = triangle->get_joint(0);
        auto* joint2 = triangle->get_joint(1);
        auto* joint3 = triangle->get_joint(2);
        vertex_info_klm_tex pt[3] =
        {
            { joint1->get_point(), vec3(), vec2() },
            { joint2->get_point(), vec3(), vec2() },
            { joint3->get_point(), vec3(), vec2() }
        };
        rose_filling_tex_coords(joint1, _texbatch, pt[0]);
        rose_filling_tex_coords(joint2, _texbatch, pt[1]);
        rose_filling_tex_coords(joint3, _texbatch, pt[2]);
        if(triangle->has_klm_coords())
            rose_filling_klm_coords(triangle, pt);
        else {
            /* simply display this triangle. */
            pt[0].klm = pt[1].klm = pt[2].klm = vec3(1.f, 0.f, 0.f);
        }
        _vertices.push_back(pt[0]);
        _vertices.push_back(pt[1]);
        _vertices.push_back(pt[2]);
    });
}

void rose_fill_batch_klm_tex::create_from_stroke(bat_stroke_batch* bat)
{
    assert(bat);
    auto& lines = bat->get_lines();
    /* deal with texture batch */
    for(auto* line : lines) {
        assert(line);
        auto* sj1 = line->get_source_joint(0);
        auto* sj2 = line->get_source_joint(1);
        assert(sj1 && sj2);
        auto* os1 = reinterpret_cast<rose_bind_info_tex*>(sj1->get_binding());
        auto* os2 = reinterpret_cast<rose_bind_info_tex*>(sj2->get_binding());
        assert(os1 && os2);
        _texbatch.add_texture(os1->img);
        _texbatch.add_texture(os2->img);
    }
    /* arrange texture batch */
    _texbatch.arrange();
    /* create vertices */
    for(auto* line : lines) {
        assert(line);
        /* point */
        auto& p1 = line->get_start_point();
        auto& p2 = line->get_end_point();
        if(line->need_recalc())
            line->calc_contour_points();
        auto* p = &line->get_contour_point(0);
        /* tex */
        auto* sj1 = line->get_source_joint(0);
        auto* sj2 = line->get_source_joint(1);
        assert(sj1 && sj2);
        auto* os1 = reinterpret_cast<rose_bind_info_tex*>(sj1->get_binding());
        auto* os2 = reinterpret_cast<rose_bind_info_tex*>(sj2->get_binding());
        assert(os1 && os2);
        float t1 = gs_clamp(linear_reparameterize(sj1->get_point(), sj2->get_point(), p1), 0.f, 1.f);
        float t2 = gs_clamp(linear_reparameterize(sj1->get_point(), sj2->get_point(), p2), 0.f, 1.f);
        assert(os1->img == os2->img);
        vec2 tex1, tex2;
        tex1.lerp(os1->tex, os2->tex, t1);
        tex2.lerp(os1->tex, os2->tex, t2);
        tex1 = rose_calc_tex_coords(os1->img, tex1, _texbatch);
        tex2 = rose_calc_tex_coords(os2->img, tex2, _texbatch);
        vertex_info_klm_tex pt[] =
        {
            { p[0], vec3(1.f, 0.f, 0.f), tex1 },
            { p[1], vec3(1.f, 0.f, 0.f), tex2 },
            { p[2], vec3(1.f, 0.f, 0.f), tex1 },
            { p[3], vec3(1.f, 0.f, 0.f), tex2 },
        };
        _vertices.push_back(pt[0]);
        _vertices.push_back(pt[1]);
        _vertices.push_back(pt[2]);
        _vertices.push_back(pt[2]);
        _vertices.push_back(pt[1]);
        _vertices.push_back(pt[3]);
    }
}

void rose_stroke_batch_coef_cr::create(bat_batch* bat)
{
    assert(bat);
    auto& lines = static_cast<bat_stroke_batch*>(bat)->get_lines();
    for(auto* line : lines) {
        assert(line);
        /* point */
        auto& p1 = line->get_start_point();
        auto& p2 = line->get_end_point();
        if(line->need_recalc())
            line->calc_contour_points();
        auto* p = &line->get_contour_point(0);
        /* coef */
        vec4 coef;
        (vec3&)coef = line->get_coef();
        coef.w = line->get_line_width() * 0.6f;    /* tune */
        /* color */
        auto* sj1 = line->get_source_joint(0);
        auto* sj2 = line->get_source_joint(1);
        assert(sj1 && sj2);
        auto* os1 = reinterpret_cast<rose_bind_info_cr*>(sj1->get_binding());
        auto* os2 = reinterpret_cast<rose_bind_info_cr*>(sj2->get_binding());
        assert(os1 && os2);
        float t1 = gs_clamp(linear_reparameterize(sj1->get_point(), sj2->get_point(), p1), 0.f, 1.f);
        float t2 = gs_clamp(linear_reparameterize(sj1->get_point(), sj2->get_point(), p2), 0.f, 1.f);
        vec4 cr1, cr2;
        cr1.lerp(os1->color, os2->color, t1);
        cr2.lerp(os1->color, os2->color, t2);
        vertex_info_coef_cr pt[] =
        {
            { p[0], coef, cr1 },
            { p[1], coef, cr2 },
            { p[2], coef, cr1 },
            { p[3], coef, cr2 },
            { line->get_head_point(), coef, cr1 },
            { line->get_tail_point(), coef, cr2 },
            { line->get_start_point(), coef, cr1 },
            { line->get_end_point(), coef, cr2 },
        };
        _vertices.push_back(pt[0]);
        _vertices.push_back(pt[1]);
        _vertices.push_back(pt[2]);
        _vertices.push_back(pt[2]);
        _vertices.push_back(pt[1]);
        _vertices.push_back(pt[3]);
        switch(line->get_head_con())
        {
        case bct_convex_wedge:
        case bct_concave_wedge:
            _vertices.push_back(pt[4]);
            _vertices.push_back(pt[0]);
            _vertices.push_back(pt[2]);
            break;
        case bct_convex_notch:
            _vertices.push_back(pt[4]);
            _vertices.push_back(pt[0]);
            _vertices.push_back(pt[6]);
            break;
        case bct_concave_notch:
            _vertices.push_back(pt[4]);
            _vertices.push_back(pt[6]);
            _vertices.push_back(pt[2]);
            break;
        default:
            break;
        }
        switch(line->get_tail_con())
        {
        case bct_convex_wedge:
        case bct_concave_wedge:
            _vertices.push_back(pt[3]);
            _vertices.push_back(pt[1]);
            _vertices.push_back(pt[5]);
            break;
        case bct_convex_notch:
            _vertices.push_back(pt[7]);
            _vertices.push_back(pt[1]);
            _vertices.push_back(pt[5]);
            break;
        case bct_concave_notch:
            _vertices.push_back(pt[3]);
            _vertices.push_back(pt[7]);
            _vertices.push_back(pt[5]);
            break;
        default:
            break;
        }
    }
}

void rose_stroke_batch_coef_cr::tracing() const
{
}

rose_stroke_batch_coef_tex::rose_stroke_batch_coef_tex(int index, render_sampler_state* ss):
    rose_batch(index)
{
    _sstate = ss;
    _srv = nullptr;
    _tex = nullptr;
}

void rose_stroke_batch_coef_tex::create(bat_batch* bat)
{
    assert(bat);
    create_vertices(bat);
}

void rose_stroke_batch_coef_tex::create_vertices(bat_batch* bat)
{
    assert(bat);
    auto& lines = static_cast<bat_stroke_batch*>(bat)->get_lines();
    /* deal with texture batch */
    for(auto* line : lines) {
        assert(line);
        auto* sj1 = line->get_source_joint(0);
        auto* sj2 = line->get_source_joint(1);
        assert(sj1 && sj2);
        auto* os1 = reinterpret_cast<rose_bind_info_tex*>(sj1->get_binding());
        auto* os2 = reinterpret_cast<rose_bind_info_tex*>(sj2->get_binding());
        assert(os1 && os2);
        _texbatch.add_texture(os1->img);
        _texbatch.add_texture(os2->img);
    }
    /* arrange texture batch */
    _texbatch.arrange();
    /* create vertices */
    create_vertices(lines, _texbatch);
}

void rose_stroke_batch_coef_tex::create_vertices(bat_lines& lines, const tex_batcher& bat)
{
    for(auto* line : lines) {
        assert(line);
        /* point */
        auto& p1 = line->get_start_point();
        auto& p2 = line->get_end_point();
        if(line->need_recalc())
            line->calc_contour_points();
        auto* p = &line->get_contour_point(0);
        /* coef */
        vec4 coef;
        (vec3&)coef = line->get_coef();
        coef.w = line->get_line_width() * 0.6f;    /* tune */
        /* tex */
        auto* sj1 = line->get_source_joint(0);
        auto* sj2 = line->get_source_joint(1);
        assert(sj1 && sj2);
        auto* os1 = reinterpret_cast<rose_bind_info_tex*>(sj1->get_binding());
        auto* os2 = reinterpret_cast<rose_bind_info_tex*>(sj2->get_binding());
        assert(os1 && os2);
        float t1 = gs_clamp(linear_reparameterize(sj1->get_point(), sj2->get_point(), p1), 0.f, 1.f);
        float t2 = gs_clamp(linear_reparameterize(sj1->get_point(), sj2->get_point(), p2), 0.f, 1.f);
        assert(os1->img == os2->img);
        vec2 tex1, tex2;
        tex1.lerp(os1->tex, os2->tex, t1);
        tex2.lerp(os1->tex, os2->tex, t2);
        tex1 = rose_calc_tex_coords(os1->img, tex1, bat);
        tex2 = rose_calc_tex_coords(os2->img, tex2, bat);
        vertex_info_coef_tex pt[] =
        {
            { p[0], coef, tex1 },
            { p[1], coef, tex2 },
            { p[2], coef, tex1 },
            { p[3], coef, tex2 },
            { line->get_head_point(), coef, tex1 },
            { line->get_tail_point(), coef, tex2 },
            { line->get_start_point(), coef, tex1 },
            { line->get_end_point(), coef, tex2 },
        };
        _vertices.push_back(pt[0]);
        _vertices.push_back(pt[1]);
        _vertices.push_back(pt[2]);
        _vertices.push_back(pt[2]);
        _vertices.push_back(pt[1]);
        _vertices.push_back(pt[3]);
        switch(line->get_head_con())
        {
        case bct_convex_wedge:
        case bct_concave_wedge:
            _vertices.push_back(pt[4]);
            _vertices.push_back(pt[0]);
            _vertices.push_back(pt[2]);
            break;
        case bct_convex_notch:
            _vertices.push_back(pt[4]);
            _vertices.push_back(pt[0]);
            _vertices.push_back(pt[6]);
            break;
        case bct_concave_notch:
            _vertices.push_back(pt[4]);
            _vertices.push_back(pt[6]);
            _vertices.push_back(pt[2]);
            break;
        default:
            break;
        }
        switch(line->get_tail_con())
        {
        case bct_convex_wedge:
        case bct_concave_wedge:
            _vertices.push_back(pt[3]);
            _vertices.push_back(pt[1]);
            _vertices.push_back(pt[5]);
            break;
        case bct_convex_notch:
            _vertices.push_back(pt[7]);
            _vertices.push_back(pt[1]);
            _vertices.push_back(pt[5]);
            break;
        case bct_concave_notch:
            _vertices.push_back(pt[3]);
            _vertices.push_back(pt[7]);
            _vertices.push_back(pt[5]);
            break;
        default:
            break;
        }
    }
}

void rose_stroke_batch_coef_tex::tracing() const
{
}

rose_stroke_batch_assoc_with_klm_tex::rose_stroke_batch_assoc_with_klm_tex(int index, rose_fill_batch_klm_tex* assoc):
    rose_stroke_batch_coef_tex(index, nullptr)
{
    assert(assoc);
    _assoc = assoc;
    _sstate = assoc->_sstate;
}

rose_stroke_batch_assoc_with_klm_tex::~rose_stroke_batch_assoc_with_klm_tex()
{
    /* we take them from associated batch, DONOT release them twice */
    _sstate = nullptr;
    _srv = nullptr;
    _tex = nullptr;
}

void rose_stroke_batch_assoc_with_klm_tex::create(bat_batch* bat)
{
    assert(bat);
    create_vertices(bat);
}

void rose_stroke_batch_assoc_with_klm_tex::create_vertices(bat_batch* bat)
{
    assert(bat && _assoc);
    __super::create_vertices(
        static_cast<bat_stroke_batch*>(bat)->get_lines(),
        _assoc->_texbatch
        );
}

void rose_bindings::clear_binding_cache()
{
    _cr_bindings.clear();
    _tex_bindings.clear();
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

void graphics_obj_entity::proceed_stroke(const painter_path& path)
{
    int size = path.size();
    for(int i = 0; i < size; )
        i = create_from_path(path, i);
}

int graphics_obj_entity::create_from_path(const painter_path& path, int start)
{
    int size = path.size();
    assert(start < size);
    auto* first = path.get_node(start);
    assert(first && first->get_tag() == painter_path::pt_moveto);
    int i = start + 1;
    if((i == size)) {
        assert(!"bad path.");
        return i;
    }
    auto* node = path.get_node(i);
    assert(node);
    if(node->get_tag() == painter_path::pt_moveto) {
        assert(!"bad path.");
        return i;
    }
    path_seg firstseg;
    switch(node->get_tag())
    {
    case painter_path::pt_lineto:
        add_line_seg(firstseg, static_cast<const painter_path::line_to_node*>(node));
        break;
    case painter_path::pt_quadto:
        add_quad_seg(firstseg, first, static_cast<const painter_path::quad_to_node*>(node));
        break;
    case painter_path::pt_cubicto:
        add_cubic_seg(firstseg, first, static_cast<const painter_path::cubic_to_node*>(node));
        break;
    default:
        assert(!"bad path.");
        return size;
    }
    assert(firstseg.first && firstseg.last);
    auto* lastline = firstseg.last;
    for(i ++; i < size; i ++) {
        auto* node = path.get_node(i);
        assert(node);
        path_seg seg;
        auto t = node->get_tag();
        if(t == painter_path::pt_moveto)
            break;
        switch(t)
        {
        case painter_path::pt_lineto:
            add_line_seg(seg, static_cast<const painter_path::line_to_node*>(node));
            break;
        case painter_path::pt_quadto:
            add_quad_seg(seg, path.get_node(i - 1), static_cast<const painter_path::quad_to_node*>(node));
            break;
        case painter_path::pt_cubicto:
            add_cubic_seg(seg, path.get_node(i - 1), static_cast<const painter_path::cubic_to_node*>(node));
            break;
        default:
            assert(!"bad path.");
            break;
        }
        lb_connect(lastline->get_next_joint(), seg.first);
        lastline = seg.last;
    }
    assert(lastline);
    if(first->get_point() == lastline->get_next_point())
        lb_connect(lastline->get_next_joint(), firstseg.first);
    else {
        auto* firstj = create_joint<lb_end_joint>(first->get_point());
        assert(firstj);
        lb_connect(firstj, firstseg.first);
    }
    return i;
}

void graphics_obj_entity::add_line_seg(path_seg& seg, const painter_path::line_to_node* node)
{
    assert(node);
    auto* joint = create_joint<lb_end_joint>(node->get_point());
    auto* line = create_line();
    lb_connect(line, joint);
    seg.first = seg.last = line;
}

void graphics_obj_entity::add_quad_seg(path_seg& seg, const painter_node* node1, const painter_path::quad_to_node* node2)
{
    assert(node1 && node2);
    vec3 para[2];
    get_quad_parameter_equation(para, node1->get_point(), node2->get_control(), node2->get_point());
    int step = get_interpolate_step(node1->get_point(), node2->get_control(), node2->get_point());
    float t, chord;
    t = chord = 1.f / (step - 1);
    auto* lastline = seg.first = create_line();
    for(int i = 1; i < step - 1; i ++, t += chord) {
        vec2 p;
        eval_quad(p, para, t);
        auto* line = create_line();
        auto* joint = create_joint<lb_end_joint>(p);
        assert(line && joint);
        lb_connect(lastline, joint);
        lb_connect(joint, line);
        lastline = line;
    }
    auto* joint = create_joint<lb_end_joint>(node2->get_point());
    assert(joint);
    lb_connect(lastline, joint);
    seg.last = lastline;
}

void graphics_obj_entity::add_cubic_seg(path_seg& seg, const painter_node* node1, const painter_path::cubic_to_node* node2)
{
    assert(node1 && node2);
    vec4 para[2];
    get_cubic_parameter_equation(para, node1->get_point(), node2->get_control1(), node2->get_control2(), node2->get_point());
    int step = get_interpolate_step(node1->get_point(), node2->get_control1(), node2->get_control2(), node2->get_point());
    float t, chord;
    t = chord = 1.f / (step - 1);
    auto* lastline = seg.first = create_line();
    for(int i = 1; i < step - 1; i ++, t += chord) {
        vec2 p;
        eval_cubic(p, para, t);
        auto* line = create_line();
        auto* joint = create_joint<lb_end_joint>(p);
        assert(line && joint);
        lb_connect(lastline, joint);
        lb_connect(joint, line);
        lastline = line;
    }
    auto* joint = create_joint<lb_end_joint>(node2->get_point());
    assert(joint);
    lb_connect(lastline, joint);
    seg.last = lastline;
}

graphics_obj::graphics_obj(float w, float h)
{
    __super::reset(new graphics_obj_entity(w, h), [](graphics_obj_entity* p) {
        assert(p);
        delete p;
    });
}

rose::rose()
{
    _nextz = 0.f;
    initialize();
}

rose::~rose()
{
    clear_batches();
    destroy_miscs();
}

void rose::draw_path(const painter_path& path)
{
    context& ctx = get_context();
    auto& brush = ctx.get_brush();
    auto& pen = ctx.get_pen();
    mat3 m;
    get_transform_recursively(m);
    painter_path p;
    p.duplicate(path);
    p.transform(m);
    prepare_fill(p, brush);
    prepare_stroke(p, pen);
}

void rose::on_draw_begin()
{
    __super::on_draw_begin();
    _nextz = 0.f;
    _bp.clear_batches();
    _bp.set_antialias(query_antialias());
    _bindings.clear_binding_cache();
}

void rose::on_draw_end()
{
    __super::on_draw_end();
    _bp.finish_batching();
    clear_batches();
    prepare_batches();
    draw_batches();
    _gocache.clear();
}

void rose::fill_non_picture_graphics_obj(graphics_obj& gfx, uint brush_tag)
{
    auto& polys = gfx->get_polygons();
    auto z = _nextz ++;
    for(auto* p : polys)
        _bp.add_non_tex_polygon(p, z, brush_tag);
}

bat_batch* rose::fill_picture_graphics_obj(graphics_obj& gfx)
{
    auto z = _nextz ++;
    return _bp.add_tex_polygons(gfx->get_polygons(), z);
}

void rose::stroke_graphics_obj(graphics_obj& gfx, uint pen_tag)
{
    float line_width = query_antialias() ? 2.4f : 2.f;
    auto z = _nextz ++;
    meta_stroke_graphics_obj(gfx, [&](lb_joint* i, lb_joint* j)-> bat_line* {
        assert(i && j);
        return _bp.create_line(i, j, line_width, z, pen_tag, false);
    });
}

template<class _addline>
void rose::meta_stroke_graphics_obj(graphics_obj& gfx, _addline fn_add)
{
    auto& lines = gfx->get_lines();
    if(lines.empty())
        return;
    auto i = lines.begin();
    auto* last = fn_add((*i)->get_prev_joint(), (*i)->get_next_joint());
    assert(last);
    last->calc_contour_points();
    auto* laststart = last;
    for(++ i; i != lines.end(); ++ i) {
        auto* line = fn_add((*i)->get_prev_joint(), (*i)->get_next_joint());
        assert(line);
        line->calc_contour_points();
        if(last->get_end_point() == line->get_start_point()) {
            last->trim_contour(*line);
            last = line;
        }
        else {
            if((last != laststart) &&
                (laststart->get_start_point() == last->get_end_point())
                )
                last->trim_contour(*laststart);
            laststart = last = line;
        }
    }
}

/*
 * Transform the coordinates like:
 * from --------+    to        +
 *      |                      |
 *      |                 -----|-----+
 *      |                      |
 *      +                      |
 */
void rose::setup_configs()
{
    float w = (float)get_width();
    float h = (float)get_height();
    _cfgs.screen = vec4(w, h, 0.f, 0.f);
    mat4 m(
        2.f / w, 0.f, -1.f, 0.f,
        0.f, -2.f / h, 1.f, 0.f,
        0.f, 0.f, 1.f, 0.f,
        0.f, 0.f, 0.f, 1.f
        );
    memcpy_s(_cfgs.mapscreen, sizeof(_cfgs.mapscreen), &m, sizeof(_cfgs.mapscreen));
    assert(_cb_configs);
    _rsys->update_buffer(_cb_configs, sizeof(rose_configs), &_cfgs);
    _rsys->set_constant_buffer(_cb_config_slot, _cb_configs, st_vertex_shader);
    _rsys->set_constant_buffer(_cb_config_slot, _cb_configs, st_pixel_shader);
}

void rose::prepare_fill(const painter_path& path, const painter_brush& brush)
{
    if(brush.get_tag() == painter_brush::none)
        return;
    if(brush.get_tag() == painter_brush::picture)
        return prepare_picture_fill(path, brush);
    graphics_obj gfx((float)get_width(), (float)get_height());
    gfx->proceed_fill(path);
    rose_paint_non_picture_brush(gfx, _bindings, brush);
    fill_non_picture_graphics_obj(gfx, brush.get_tag());
    _gocache.push_back(gfx);
    if(!query_antialias())
        return;
    /* anti-aliasing */
    auto z = _nextz ++;
    graphics_obj gfxaa((float)get_width(), (float)get_height());
    gfxaa->proceed_stroke(path);
    rose_paint_non_picture_brush(gfxaa, _bindings, brush);
    auto get_relevant_tag = [](uint brush_tag)->uint {
        switch(brush_tag)
        {
        case painter_brush::none:
            return painter_pen::none;
        case painter_brush::solid:
            return painter_pen::solid;
        case painter_brush::picture:
            return painter_pen::picture;
        default:
            assert(!"unexpected tag.");
            return painter_pen::none;
        }
    };
    uint pen_tag = get_relevant_tag(brush.get_tag());
    assert(pen_tag != painter_pen::picture);
    meta_stroke_graphics_obj(gfxaa, [this, &z, &pen_tag](lb_joint* i, lb_joint* j)-> bat_line* {
        assert(i && j);
        static const float aa_width = 1.4f;
        return _bp.create_line(i, j, aa_width, z, pen_tag, true);
    });
    _gocache.push_back(gfxaa);
}

void rose::prepare_picture_fill(const painter_path& path, const painter_brush& brush)
{
    assert(brush.get_tag() == painter_brush::picture);
    rectf bound;
    path.get_boundary_box(bound);
    graphics_obj gfx((float)get_width(), (float)get_height());
    gfx->proceed_fill(path);
    rose_paint_picture_brush(gfx, bound, _bindings.get_tex_bindings(), brush);
    auto* fill_bat = fill_picture_graphics_obj(gfx);
    assert(fill_bat);
    _gocache.push_back(gfx);
    if(!query_antialias())
        return;
    /* anti-aliasing */
    auto z = _nextz ++;
    graphics_obj gfxaa((float)get_width(), (float)get_height());
    gfxaa->proceed_stroke(path);
    rose_paint_picture_brush(gfxaa, bound, _bindings.get_tex_bindings(), brush);
    auto* stroke_bat = _bp.find_associated_tex_stroke_batch(fill_bat);
    assert(stroke_bat);
    meta_stroke_graphics_obj(gfxaa, [this, &z, &stroke_bat](lb_joint* i, lb_joint* j)-> bat_line* {
        assert(i && j);
        static const float aa_width = 1.4f;
        auto* line = bat_line::create_line(i, j, aa_width, z, painter_pen::picture, true);
        assert(line);
        stroke_bat->add_line(line);
        return line;
    });
    _gocache.push_back(gfxaa);
}

void rose::prepare_stroke(const painter_path& path, const painter_pen& pen)
{
    if(pen.get_tag() == painter_pen::none)
        return;
    graphics_obj gfx((float)get_width(), (float)get_height());
    gfx->proceed_stroke(path);
    rose_paint_pen(gfx, path, _bindings, pen);
    stroke_graphics_obj(gfx, pen.get_tag());
    _gocache.push_back(gfx);
}

rose_batch* rose::create_fill_batch_cr(int index)
{
    auto* ptr = new rose_fill_batch_cr(index);
    ptr->set_vertex_shader(_vsf_cr);
    ptr->set_pixel_shader(_psf_cr);
    ptr->set_vertex_format(_vf_cr);
    _batches.push_back(ptr);
    return ptr;
}

rose_batch* rose::create_fill_batch_klm_cr(int index)
{
    auto* ptr = new rose_fill_batch_klm_cr(index);
    ptr->set_vertex_shader(_vsf_klm_cr);
    ptr->set_pixel_shader(_psf_klm_cr);
    ptr->set_vertex_format(_vf_klm_cr);
    _batches.push_back(ptr);
    return ptr;
}

rose_batch* rose::create_fill_batch_klm_tex(int index)
{
    auto* sstate = acquire_default_sampler_state();
    auto* ptr = new rose_fill_batch_klm_tex(index, sstate);
    ptr->set_vertex_shader(_vsf_klm_tex);
    ptr->set_pixel_shader(_psf_klm_tex);
    ptr->set_vertex_format(_vf_klm_tex);
    _batches.push_back(ptr);
    return ptr;
}

rose_batch* rose::create_stroke_batch_cr(int index)
{
    if(!query_antialias())
        return create_fill_batch_cr(index);
    auto* ptr = new rose_stroke_batch_coef_cr(index);
    ptr->set_vertex_shader(_vss_coef_cr);
    ptr->set_pixel_shader(_pss_coef_cr);
    ptr->set_vertex_format(_vf_coef_cr);
    _batches.push_back(ptr);
    return ptr;
}

rose_batch* rose::create_stroke_batch_tex(int index)
{
    if(!query_antialias())
        return create_fill_batch_klm_tex(index);
    auto* sstate = acquire_default_sampler_state();
    auto* ptr = new rose_stroke_batch_coef_tex(index, sstate);
    ptr->set_vertex_shader(_vss_coef_tex);
    ptr->set_pixel_shader(_pss_coef_tex);
    ptr->set_vertex_format(_vf_coef_tex);
    _batches.push_back(ptr);
    return ptr;
}

rose_batch* rose::create_stroke_batch_assoc(int index, rose_fill_batch_klm_tex* assoc)
{
    assert(assoc);
    auto* ptr = new rose_stroke_batch_assoc_with_klm_tex(index, assoc);
    ptr->set_vertex_shader(_vss_coef_tex);
    ptr->set_pixel_shader(_pss_coef_tex);
    ptr->set_vertex_format(_vf_coef_tex);
    _batches.push_back(ptr);
    return ptr;
}

void rose::clear_batches()
{
    for(auto* p : _batches) { delete p; }
    _batches.clear();
}

void rose::prepare_batches()
{
    auto& batches = _bp.get_batches();
    int size = (int)batches.size();
    for(int i = 0; i < size; i ++) {
        auto* p = batches.at(i);
        auto t = p->get_type();
        rose_batch* bat = nullptr;
        switch(t)
        {
        case bf_cr:
            bat = create_fill_batch_cr(i);
            break;
        case bf_klm_cr:
            bat = create_fill_batch_klm_cr(i);
            break;
        case bf_klm_tex:
            bat = create_fill_batch_klm_tex(i);
            if(query_antialias()) {
                assert(bat);
                bat->create(p);
                bat->buffering(_rsys);
                /* should have an associated stroke batch after. */
                p = batches.at(++ i);
                assert(i < size);
                assert(p->get_type() == bs_coef_tex);
                bat = create_stroke_batch_assoc(i, static_cast<rose_fill_batch_klm_tex*>(bat));
            }
            break;
        case bs_coef_cr:
            bat = create_stroke_batch_cr(i);
            break;
        case bs_coef_tex:
            bat = create_stroke_batch_tex(i);
            break;
        }
        assert(bat);
        bat->create(p);
        bat->buffering(_rsys);
    }
}

void rose::draw_batches()
{
    for(auto* p : _batches) {
        p->draw(_rsys);
#if (defined(_DEBUG) || defined(DEBUG)) && defined(_GS_DEBUG_VERBOSE)
        p->tracing();
#endif
    }
}

void rose_paint_non_picture_brush(graphics_obj& gfx, rose_bindings& bindings, const painter_brush& brush)
{
    auto t = brush.get_tag();
    assert(t != painter_brush::picture);
    switch(t)
    {
    case painter_brush::solid:
        return rose_paint_solid_brush(gfx, bindings.get_cr_bindings(), brush);
    }
}

void rose_paint_solid_brush(graphics_obj& gfx, rose_bind_list_cr& bind_cache, const painter_brush& brush)
{
    assert(brush.get_tag() == painter_brush::solid);
    bind_cache.push_back(rose_bind_info_cr());
    auto& binding = bind_cache.back();
    auto& cr = brush.get_color();
    binding.color.x = (float)cr.red / 255.f;
    binding.color.y = (float)cr.green / 255.f;
    binding.color.z = (float)cr.blue / 255.f;
    binding.color.w = (float)cr.alpha / 255.f;
    auto& joints = gfx->get_joints();
    for(auto* p : joints) {
        assert(p);
        p->set_binding(&binding);
    }
}

static void rose_paint_picture_joint(lb_joint* joint, rose_bind_list_tex& bind_cache, texture2d* img, const mat3& m)
{
    assert(joint && img);
    vec3 tex;
    const vec2& pt = joint->get_point();
    tex.multiply(vec3(pt.x, pt.y, 1.f), m);
    bind_cache.push_back(rose_bind_info_tex());
    auto& binding = bind_cache.back();
    binding.img = img;
    binding.tex = vec2(tex.x / tex.z, tex.y / tex.z);
    joint->set_binding(&binding);
}

void rose_paint_picture_brush(graphics_obj& gfx, const rectf& bound, rose_bind_list_tex& bind_cache, const painter_brush& brush)
{
    assert(brush.get_tag() == painter_brush::picture);
    auto& ext = brush.get_extra();
    const auto* pd = static_cast<const painter_picture_data*>(ext.get());
    auto* img = pd->get_image();
    assert(img);
    const rectf& rc = pd->get_src_rect();
    mat3 m;
    m.translation(-bound.left, -bound.top);
    m.multiply(mat3().scaling(rc.width() / bound.width(), rc.height() / bound.height()));
    m.multiply(mat3().translation(rc.left, rc.top));
    auto& joints = gfx->get_joints();
    for(auto* p : joints) {
        assert(p);
        rose_paint_picture_joint(p, bind_cache, img, m);
    }
}

void rose_paint_pen(graphics_obj& gfx, const painter_path& path, rose_bindings& bindings, const painter_pen& pen)
{
    auto t = pen.get_tag();
    switch(t)
    {
    case painter_pen::solid:
        return rose_paint_solid_pen(gfx, bindings.get_cr_bindings(), pen);
    case painter_pen::picture:
        {
            rectf bound;
            path.get_boundary_box(bound);
            return rose_paint_picture_pen(gfx, bound, bindings.get_tex_bindings(), pen);
        }
    }
}

void rose_paint_solid_pen(graphics_obj& gfx, rose_bind_list_cr& bind_cache, const painter_pen& pen)
{
    assert(pen.get_tag() == painter_pen::solid);
    bind_cache.push_back(rose_bind_info_cr());
    auto& binding = bind_cache.back();
    auto& cr = pen.get_color();
    binding.color.x = (float)cr.red / 255.f;
    binding.color.y = (float)cr.green / 255.f;
    binding.color.z = (float)cr.blue / 255.f;
    binding.color.w = (float)cr.alpha / 255.f;
    auto& joints = gfx->get_joints();
    for(auto* p : joints) {
        assert(p);
        p->set_binding(&binding);
    }
}

void rose_paint_picture_pen(graphics_obj& gfx, const rectf& bound, rose_bind_list_tex& bind_cache, const painter_pen& pen)
{
    assert(pen.get_tag() == painter_pen::picture);
    auto& ext = pen.get_extra();
    const auto* pd = static_cast<const painter_picture_data*>(ext.get());
    auto* img = pd->get_image();
    assert(img);
    const rectf& rc = pd->get_src_rect();
    mat3 m;
    m.translation(-bound.left, -bound.top);
    m.multiply(mat3().scaling(rc.width() / bound.width(), rc.height() / bound.height()));
    m.multiply(mat3().translation(rc.left, rc.top));
    auto& joints = gfx->get_joints();
    for(auto* p : joints) {
        assert(p);
        rose_paint_picture_joint(p, bind_cache, img, m);
    }
}

__ariel_end__
