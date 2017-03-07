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

#include <ariel/rose.h>
#include <ariel/rendersysd3d11.h>

__ariel_begin__

template<class stream_type>
int rose_batch::template_buffering(stream_type& stm, rendersys* rsys)
{
    assert(rsys);
    assert(!_vertex_buffer);
    int c = (int)stm.size();
    _vertex_buffer = rsys->create_vertex_buffer(sizeof(stream_type::value_type), c, false, false, D3D11_USAGE_DEFAULT, &stm.front());
    assert(_vertex_buffer);
    return c;
}

int rose_fill_batch_cr::buffering(rendersys* rsys)
{
    return template_buffering(_vertices, rsys);
}

void rose_fill_batch_cr::draw(rendersys* rsys)
{
    assert(rsys);
    setup_vs_and_ps(rsys);
    setup_vf_and_topology(rsys, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    rsys->set_vertex_buffer(_vertex_buffer, sizeof(vertex_info_cr), 0);
    int c = (int)_vertices.size();
    assert(c % 3 == 0);
    rsys->draw(c, 0);
}

int rose_fill_batch_klm_cr::buffering(rendersys* rsys)
{
    return template_buffering(_vertices, rsys);
}

void rose_fill_batch_klm_cr::draw(rendersys* rsys)
{
    assert(rsys);
    setup_vs_and_ps(rsys);
    setup_vf_and_topology(rsys, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    rsys->set_vertex_buffer(_vertex_buffer, sizeof(vertex_info_klm_cr), 0);
    int c = (int)_vertices.size();
    assert(c % 3 == 0);
    rsys->draw(c, 0);
}

int rose_fill_batch_tex::buffering(rendersys* rsys)
{
    return template_buffering(_vertices, rsys);
}

void rose_fill_batch_tex::draw(rendersys* rsys)
{
    assert(rsys);
    setup_vs_and_ps(rsys);
    setup_vf_and_topology(rsys, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    rsys->set_vertex_buffer(_vertex_buffer, sizeof(vertex_info_tex), 0);
    int c = (int)_vertices.size();
    assert(c % 3 == 0);
    rsys->draw(c, 0);
}

int rose_fill_batch_klm_tex::buffering(rendersys* rsys)
{
    return template_buffering(_vertices, rsys);
}

void rose_fill_batch_klm_tex::draw(rendersys* rsys)
{
    assert(rsys);
    setup_vs_and_ps(rsys);
    setup_vf_and_topology(rsys, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    rsys->set_vertex_buffer(_vertex_buffer, sizeof(vertex_info_klm_tex), 0);
    int c = (int)_vertices.size();
    assert(c % 3 == 0);
    rsys->draw(c, 0);
}

int rose_stroke_batch_coef_cr::buffering(rendersys* rsys)
{
    return template_buffering(_vertices, rsys);
}

void rose_stroke_batch_coef_cr::draw(rendersys* rsys)
{
    assert(rsys);
    setup_vs_and_ps(rsys);
    setup_vf_and_topology(rsys, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    rsys->set_vertex_buffer(_vertex_buffer, sizeof(vertex_info_coef_cr), 0);
    int c = (int)_vertices.size();
    assert(c % 3 == 0);
    rsys->draw(c, 0);
}

int rose_stroke_batch_coef_tex::buffering(rendersys* rsys)
{
    return template_buffering(_vertices, rsys);
}

void rose_stroke_batch_coef_tex::draw(rendersys* rsys)
{
    assert(rsys);
    setup_vs_and_ps(rsys);
    setup_vf_and_topology(rsys, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    rsys->set_vertex_buffer(_vertex_buffer, sizeof(vertex_info_coef_tex), 0);
    int c = (int)_vertices.size();
    assert(c % 3 == 0);
    rsys->draw(c, 0);
}

void rose::setup(rendersys* rsys)
{
    assert(rsys);
    _rsys = rsys;
    rendersys::vertex_format_desc descf_cr[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    rendersys::vertex_format_desc descf_klm_cr[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    rendersys::vertex_format_desc descs_coef_cr[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    rendersys::create_shader_context ctx;
    /* create shader cr */
    _rsys->begin_create_shader(ctx, _t("rose.hlsl"), _t("rose_vsf_cr"), _t("vs_4_0"), 0);
    _vsf_cr = _rsys->create_vertex_shader(ctx);
    assert(_vsf_cr);
    _vf_cr = _rsys->create_vertex_format(ctx, descf_cr, _countof(descf_cr));
    assert(_vf_cr);
    _rsys->end_create_shader(ctx);
    _rsys->begin_create_shader(ctx, _t("rose.hlsl"), _t("rose_psf_cr"), _t("ps_4_0"), 0);
    _psf_cr = _rsys->create_pixel_shader(ctx);
    assert(_psf_cr);
    _rsys->end_create_shader(ctx);
    /* create shader klm cr */
    _rsys->begin_create_shader(ctx, _t("rose.hlsl"), _t("rose_vsf_klm_cr"), _t("vs_4_0"), 0);
    _vsf_klm_cr = _rsys->create_vertex_shader(ctx);
    assert(_vsf_klm_cr);
    _vf_klm_cr = _rsys->create_vertex_format(ctx, descf_klm_cr, _countof(descf_klm_cr));
    assert(_vf_klm_cr);
    _rsys->end_create_shader(ctx);
    _rsys->begin_create_shader(ctx, _t("rose.hlsl"), _t("rose_psf_klm_cr"), _t("ps_4_0"), 0);
    _psf_klm_cr = _rsys->create_pixel_shader(ctx);
    assert(_psf_klm_cr);
    _rsys->end_create_shader(ctx);
    /* create shader coef cr */
    _rsys->begin_create_shader(ctx, _t("rose.hlsl"), _t("rose_vss_coef_cr"), _t("vs_4_0"), 0);
    _vss_coef_cr = _rsys->create_vertex_shader(ctx);
    assert(_vss_coef_cr);
    _vf_coef_cr = _rsys->create_vertex_format(ctx, descs_coef_cr, _countof(descs_coef_cr));
    assert(_vf_coef_cr);
    _rsys->end_create_shader(ctx);
    _rsys->begin_create_shader(ctx, _t("rose.hlsl"), _t("rose_pss_coef_cr"), _t("ps_4_0"), 0);
    _pss_coef_cr = _rsys->create_pixel_shader(ctx);
    assert(_pss_coef_cr);
    _rsys->end_create_shader(ctx);
    /* create cb_configs */
    assert(!_cb_configs);
    _cb_configs = _rsys->create_constant_buffer(pack_cb_size<rose_configs>(), false, true);
    assert(_cb_configs);
    setup_configs();
}

void rose::initialize()
{
    _vsf_cr = 0;
    _vsf_klm_cr = 0;
    _vsf_tex = 0;
    _vsf_klm_tex = 0;
    _psf_cr = 0;
    _psf_klm_cr = 0;
    _psf_tex = 0;
    _psf_klm_tex = 0;
    _vf_cr = 0;
    _vf_klm_cr = 0;
    _vf_tex = 0;
    _vf_klm_tex = 0;
    _vss_coef_cr = 0;
    _vss_coef_tex = 0;
    _pss_coef_cr = 0;
    _pss_coef_tex = 0;
    _vf_coef_cr = 0;
    _vf_coef_tex = 0;
    _cb_configs = 0;
    _cb_config_slot = 0;
}

template<class c>
static void release_any(c& cptr)
{
    if(cptr) {
        cptr->Release();
        cptr = 0;
    }
}

void rose::destroy_plugin()
{
    release_constant_buffer(_cb_configs);
    release_any(_vf_cr);
    release_any(_vf_klm_cr);
    release_any(_vf_tex);
    release_any(_vf_klm_tex);
    release_any(_vsf_cr);
    release_any(_vsf_klm_cr);
    release_any(_vsf_tex);
    release_any(_vsf_klm_tex);
    release_any(_psf_cr);
    release_any(_psf_klm_cr);
    release_any(_psf_tex);
    release_any(_psf_klm_tex);
    release_any(_vf_coef_cr);
    release_any(_vf_coef_tex);
    release_any(_vss_coef_cr);
    release_any(_pss_coef_cr);
    release_any(_vss_coef_tex);
    release_any(_pss_coef_tex);
    _cb_configs = 0;
}

__ariel_end__
