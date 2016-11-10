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

#include <ariel/rose.h>
#include <gslib/error.h>

#if use_rendersys_d3d_11
#include <ariel/rosed3d11.cpp>
#endif

__ariel_begin__

rose_batch::rose_batch()
{
    _vertex_shader = 0;
    _pixel_shader = 0;
    _vertex_format = 0;
    _vertex_buffer = 0;
}

rose_batch::~rose_batch()
{
    if(_vertex_buffer) {
        release_vertex_buffer(_vertex_buffer);
        _vertex_buffer = 0;
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
    auto& rtr = static_cast<bat_fill_batch*>(bat)->get_rtree();
    rtr.for_each([this](bat_rtree_entity* ent) {
        assert(ent);
        if(!ent->get_bind_arg())
            return;
        auto* triangle = reinterpret_cast<bat_triangle*>(ent->get_bind_arg());
        assert(triangle);
        vertex_info_cr pt[3] =
        {
            { triangle->get_point(0), reinterpret_cast<rose_bind_info*>(triangle->get_lb_binding(0))->color },
            { triangle->get_point(1), reinterpret_cast<rose_bind_info*>(triangle->get_lb_binding(1))->color },
            { triangle->get_point(2), reinterpret_cast<rose_bind_info*>(triangle->get_lb_binding(2))->color }
        };
        _vertices.push_back(pt[0]);
        _vertices.push_back(pt[1]);
        _vertices.push_back(pt[2]);
    });
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
            { joint1->get_point(), vec3(), reinterpret_cast<rose_bind_info*>(joint1->get_binding())->color },
            { joint2->get_point(), vec3(), reinterpret_cast<rose_bind_info*>(joint2->get_binding())->color },
            { joint3->get_point(), vec3(), reinterpret_cast<rose_bind_info*>(joint3->get_binding())->color }
        };
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

rose::rose()
{
    _nextz = 0;
    initialize();
}

rose::~rose()
{
    clear_batches();
    destroy_plugin();
}

void rose::draw_path(const painter_path& path)
{
    context& ctx = get_context();
    auto& brush = ctx.get_brush();
    auto& pen = ctx.get_pen();
    prepare_fill(path, brush);
    prepare_stroke(path, pen);
}

void rose::on_draw_begin()
{
    _nextz = 0;
    _bp.clear_batches();
    _bindings.clear();
}

void rose::on_draw_end()
{
    _bp.finish_batching();
    clear_batches();
    prepare_batches();
    draw_batches();
    //draw_test_lines();
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
    _rsys->set_constant_buffers(_cb_config_slot, _cb_configs, st_vertex_shader);
    _rsys->set_constant_buffers(_cb_config_slot, _cb_configs, st_pixel_shader);
}

void rose::prepare_fill(const painter_path& path, const painter_brush& brush)
{
    if(brush.get_tag() == painter_brush::null)
        return;
    using namespace pink;
    painter_path cspath;
    painter_helper::transform(cspath, path,
        painter_helper::merge_straight_line | painter_helper::reduce_short_line | painter_helper::reduce_straight_curve
        );
    loop_blinn_processor lbp((float)get_width(), (float)get_height());
    lbp.proceed(cspath);
    rose_paint_brush(lbp, _bindings, brush);
    auto& polys = lbp.get_polygons();
    for(auto* p : polys)
        _bp.add_polygon(p, _nextz ++);
}

struct my_line_struct
{
    vec2 p1, p2;
    vec3 coef;
    void setup_coef()
    {
        pink::get_linear_coefficient(coef, p1, vec2().sub(p2, p1));
        coef.normalize();
    }
};

typedef list<my_line_struct> my_line_list;
static my_line_list g_linelist;

void rose::prepare_stroke(const painter_path& path, const painter_pen& pen)
{
    if(pen.get_tag() == painter_pen::null)
        return;
    // test
    if(path.size() != 2)
        return;
    auto* p1 = path.get_node(0);
    auto* p2 = path.get_node(1);
    my_line_struct ls;
    ls.p1 = p1->get_point();
    ls.p2 = p2->get_point();
    ls.setup_coef();
    g_linelist.clear();
    g_linelist.push_back(ls);
}

rose_batch* rose::create_fill_batch_cr()
{
    auto* ptr = gs_new(rose_fill_batch_cr);
    ptr->set_vertex_shader(_vsf_cr);
    ptr->set_pixel_shader(_psf_cr);
    ptr->set_vertex_format(_vf_cr);
    _batches.push_back(ptr);
    return ptr;
}

rose_batch* rose::create_fill_batch_klm_cr()
{
    auto* ptr = gs_new(rose_fill_batch_klm_cr);
    ptr->set_vertex_shader(_vsf_klm_cr);
    ptr->set_pixel_shader(_psf_klm_cr);
    ptr->set_vertex_format(_vf_klm_cr);
    _batches.push_back(ptr);
    return ptr;
}

void rose::clear_batches()
{
    for(auto* p : _batches) { gs_del(rose_batch, p); }
    _batches.clear();
}

void rose::prepare_batches()
{
    auto& batches = _bp.get_batches();
    for(auto* p : batches) {
        rose_batch* bat = 0;
        auto t = p->get_type();
        switch(t)
        {
        case bf_cr:
            bat = create_fill_batch_cr();
            break;
        case bf_klm_cr:
            bat = create_fill_batch_klm_cr();
            break;
        }
        if(!bat)
            continue;
        assert(bat);
        bat->create(p);
        bat->buffering(_rsys);
    }
}

void rose::draw_batches()
{
    for(auto* p : _batches) {
        p->draw(_rsys);
        p->tracing();
    }
}

void rose::draw_test_lines()
{
    if(g_linelist.size() != 1)
        return;
    auto& line = g_linelist.front();
    _rsys->set_vertex_shader(_vss_coef_cr);
    _rsys->set_pixel_shader(_pss_coef_cr);
    _rsys->set_vertex_format(_vf_coef_cr);
    _rsys->set_render_option(opt_primitive_topology, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    auto offset_line = [](const vec2& p1, const vec2& p2, vec2& p3, vec2& p4, float dist) {
        vec2 d;
        d.sub(p2, p1);
        vec2 xd(d.y, -d.x);
        xd.normalize();
        xd.scale(dist);
        p3.add(p1, xd);
        p4.add(p2, xd);
    };

    float width = 2.f;
    vec2 p1, p2, p3, p4, p5, p6, p7, p8;
    offset_line(line.p1, line.p2, p1, p2, width * 0.5f); // re
    offset_line(line.p1, line.p2, p3, p4, 2.f);
    offset_line(line.p1, line.p2, p5, p6, -width * 0.5f); // ct
    offset_line(line.p1, line.p2, p7, p8, -2.f);

    vec4 coef1(line.coef.x, line.coef.y, line.coef.z, width * 0.75f);
    vec4 coef2(-line.coef.x, -line.coef.y, -line.coef.z, 1.4f);

    vertex_info_coef_cr pt[] =
    {
        { p1, coef1, vec4(1.f, 0.f, 0.f, 1.f) },
        { p2, coef1, vec4(1.f, 0.f, 0.f, 1.f) },
        { p5, coef1, vec4(1.f, 0.f, 0.f, 1.f) },

        { p5, coef1, vec4(1.f, 0.f, 0.f, 1.f) },
        { p2, coef1, vec4(1.f, 0.f, 0.f, 1.f) },
        { p6, coef1, vec4(1.f, 0.f, 0.f, 1.f) },
    };

//     vertex_info_coef_cr pt[] =
//     {
//         //{ line.p1, coef1, vec4(1.f, 0.f, 0.f, 1.f) },
//         //{ line.p2, coef1, vec4(1.f, 0.f, 0.f, 1.f) },
//         { p1, coef1, vec4(1.f, 0.f, 0.f, 1.f) },
//         { p2, coef1, vec4(1.f, 0.f, 0.f, 1.f) },
//         //{ p3, coef1, vec4(1.f, 0.f, 0.f, 1.f) },
//         //{ p4, coef1, vec4(1.f, 0.f, 0.f, 1.f) },
//         { p5, coef1, vec4(1.f, 0.f, 0.f, 1.f) },
//         { p6, coef1, vec4(1.f, 0.f, 0.f, 1.f) },
//         //{ p7, coef1, vec4(1.f, 0.f, 0.f, 1.f) },
//         //{ p8, coef1, vec4(1.f, 0.f, 0.f, 1.f) },
//     };
    auto* vb = _rsys->create_vertex_buffer(sizeof(vertex_info_coef_cr), _countof(pt), false, false, D3D11_USAGE_DEFAULT, pt);
    _rsys->set_vertex_buffer(vb, sizeof(vertex_info_coef_cr), 0);
    _rsys->draw(_countof(pt), 0);
}

void rose_paint_brush(loop_blinn_processor& lbp, rose_bind_list& bind_cache, const painter_brush& brush)
{
    auto t = brush.get_tag();
    switch(t)
    {
    case painter_brush::solid:
        return rose_paint_solid_brush(lbp, bind_cache, brush);
    }
}

void rose_paint_solid_brush(loop_blinn_processor& lbp, rose_bind_list& bind_cache, const painter_brush& brush)
{
    assert(brush.get_tag() == painter_brush::solid);
    bind_cache.push_back(rose_bind_info());
    auto& binding = bind_cache.back();
    auto& cr = brush.get_color();
    binding.color.x = (float)cr.red / 255.f;
    binding.color.y = (float)cr.green / 255.f;
    binding.color.z = (float)cr.blue / 255.f;
    binding.color.w = (float)cr.alpha / 255.f;
    auto& joints = lbp.get_joints();
    for(auto* p : joints) {
        assert(p);
        p->set_binding(&binding);
    }
}

__ariel_end__
