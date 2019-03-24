/*
 * Copyright (c) 2016-2018 lymastee, All rights reserved.
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

#include "rose_psf_cr.h"
#include "rose_psf_klm_cr.h"
#include "rose_psf_klm_tex.h"
#include "rose_pss_coef_cr.h"
#include "rose_pss_coef_tex.h"
#include "rose_vsf_cr.h"
#include "rose_vsf_klm_cr.h"
#include "rose_vsf_klm_tex.h"
#include "rose_vss_coef_cr.h"
#include "rose_vss_coef_tex.h"

__ariel_begin__

template<class c>
static void release_any(c& cptr)
{
    if(cptr) {
        cptr->Release();
        cptr = nullptr;
    }
}

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

int rose_fill_batch_klm_tex::buffering(rendersys* rsys)
{
    assert(!_tex && !_srv);
    _tex = _texbatch.create_texture(rsys);
    assert(_tex);
    _srv = rsys->create_shader_resource_view(convert_to_resource(_tex));
    assert(_srv);
    return template_buffering(_vertices, rsys);
}

void rose_fill_batch_klm_tex::draw(rendersys* rsys)
{
    assert(rsys);
    setup_vs_and_ps(rsys);
    setup_vf_and_topology(rsys, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    rsys->set_vertex_buffer(_vertex_buffer, sizeof(vertex_info_klm_tex), 0);
    rsys->set_sampler_state(0, _sstate, st_pixel_shader);
    rsys->set_shader_resource(0, _srv, st_pixel_shader);
    int c = (int)_vertices.size();
    assert(c % 3 == 0);
    rsys->draw(c, 0);
}

void rose_fill_batch_klm_tex::destroy()
{
    release_any(_sstate);
    release_any(_tex);
    release_any(_srv);
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
    assert(!_tex && !_srv);
    _tex = _texbatch.create_texture(rsys);
    assert(_tex);
    _srv = rsys->create_shader_resource_view(convert_to_resource(_tex));
    assert(_srv);
    return template_buffering(_vertices, rsys);
}

void rose_stroke_batch_coef_tex::draw(rendersys* rsys)
{
    assert(rsys);
    setup_vs_and_ps(rsys);
    setup_vf_and_topology(rsys, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    rsys->set_vertex_buffer(_vertex_buffer, sizeof(vertex_info_coef_tex), 0);
    rsys->set_sampler_state(0, _sstate, st_pixel_shader);
    rsys->set_shader_resource(0, _srv, st_pixel_shader);
    int c = (int)_vertices.size();
    assert(c % 3 == 0);
    rsys->draw(c, 0);
}

void rose_stroke_batch_coef_tex::destroy()
{
    release_any(_sstate);
    release_any(_tex);
    release_any(_srv);
}

int rose_stroke_batch_assoc_with_klm_tex::buffering(rendersys* rsys)
{
    assert(!_tex && !_srv);
    assert(_assoc);
    _tex = _assoc->_tex;
    _srv = _assoc->_srv;
    assert(_tex && _srv);
    return template_buffering(_vertices, rsys);
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
    rendersys::vertex_format_desc descf_klm_tex[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    rendersys::vertex_format_desc descs_coef_tex[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    /* create shader cr */
    _vsf_cr = _rsys->create_vertex_shader(g_rose_vsf_cr, sizeof(g_rose_vsf_cr));
    assert(_vsf_cr);
    _vf_cr = _rsys->create_vertex_format(g_rose_vsf_cr, sizeof(g_rose_vsf_cr), descf_cr, _countof(descf_cr));
    assert(_vf_cr);
    _psf_cr = _rsys->create_pixel_shader(g_rose_psf_cr, sizeof(g_rose_psf_cr));
    assert(_psf_cr);
    /* create shader klm cr */
    _vsf_klm_cr = _rsys->create_vertex_shader(g_rose_vsf_klm_cr, sizeof(g_rose_vsf_klm_cr));
    assert(_vsf_klm_cr);
    _vf_klm_cr = _rsys->create_vertex_format(g_rose_vsf_klm_cr, sizeof(g_rose_vsf_klm_cr), descf_klm_cr, _countof(descf_klm_cr));
    assert(_vf_klm_cr);
    _psf_klm_cr = _rsys->create_pixel_shader(g_rose_psf_klm_cr, sizeof(g_rose_psf_klm_cr));
    assert(_psf_klm_cr);
    /* create shader coef cr */
    _vss_coef_cr = _rsys->create_vertex_shader(g_rose_vss_coef_cr, sizeof(g_rose_vss_coef_cr));
    assert(_vss_coef_cr);
    _vf_coef_cr = _rsys->create_vertex_format(g_rose_vss_coef_cr, sizeof(g_rose_vss_coef_cr), descs_coef_cr, _countof(descs_coef_cr));
    assert(_vf_coef_cr);
    _pss_coef_cr = _rsys->create_pixel_shader(g_rose_pss_coef_cr, sizeof(g_rose_pss_coef_cr));
    assert(_pss_coef_cr);
    /* create shader klm tex */
    _vsf_klm_tex = _rsys->create_vertex_shader(g_rose_vsf_klm_tex, sizeof(g_rose_vsf_klm_tex));
    assert(_vsf_klm_tex);
    _vf_klm_tex = _rsys->create_vertex_format(g_rose_vsf_klm_tex, sizeof(g_rose_vsf_klm_tex), descf_klm_tex, _countof(descf_klm_tex));
    assert(_vf_klm_tex);
    _psf_klm_tex = _rsys->create_pixel_shader(g_rose_psf_klm_tex, sizeof(g_rose_psf_klm_tex));
    assert(_psf_klm_tex);
    /* create shader coef tex */
    _vss_coef_tex = _rsys->create_vertex_shader(g_rose_vss_coef_tex, sizeof(g_rose_vss_coef_tex));
    assert(_vss_coef_tex);
    _vf_coef_tex = _rsys->create_vertex_format(g_rose_vss_coef_tex, sizeof(g_rose_vss_coef_tex), descs_coef_tex, _countof(descs_coef_tex));
    assert(_vf_coef_tex);
    _pss_coef_tex = _rsys->create_pixel_shader(g_rose_pss_coef_tex, sizeof(g_rose_pss_coef_tex));
    assert(_pss_coef_tex);
    /* create sampler state */
    _sampler_state = _rsys->create_sampler_state(ssf_linear);
    assert(_sampler_state);
    /* create cb_configs */
    assert(!_cb_configs);
    _cb_configs = _rsys->create_constant_buffer(pack_cb_size<rose_configs>(), false, true);
    assert(_cb_configs);
    setup_configs();
}

void rose::initialize()
{
    _vsf_cr = nullptr;
    _vsf_klm_cr = nullptr;
    _vsf_klm_tex = nullptr;
    _psf_cr = nullptr;
    _psf_klm_cr = nullptr;
    _psf_klm_tex = nullptr;
    _vf_cr = nullptr;
    _vf_klm_cr = nullptr;
    _vf_klm_tex = nullptr;
    _vss_coef_cr = nullptr;
    _vss_coef_tex = nullptr;
    _pss_coef_cr = nullptr;
    _pss_coef_tex = nullptr;
    _vf_coef_cr = nullptr;
    _vf_coef_tex = nullptr;
    _cb_configs = nullptr;
    _sampler_state = nullptr;
    _cb_config_slot = 0;
}

void rose::destroy_miscs()
{
    release_constant_buffer(_cb_configs);
    release_any(_sampler_state);
    release_any(_vf_cr);
    release_any(_vf_klm_cr);
    release_any(_vf_klm_tex);
    release_any(_vsf_cr);
    release_any(_vsf_klm_cr);
    release_any(_vsf_klm_tex);
    release_any(_psf_cr);
    release_any(_psf_klm_cr);
    release_any(_psf_klm_tex);
    release_any(_vf_coef_cr);
    release_any(_vf_coef_tex);
    release_any(_vss_coef_cr);
    release_any(_pss_coef_cr);
    release_any(_vss_coef_tex);
    release_any(_pss_coef_tex);
    _cb_configs = 0;
}

render_sampler_state* rose::acquire_default_sampler_state()
{
    assert(_sampler_state);
    _sampler_state->AddRef();
    return _sampler_state;
}

__ariel_end__
