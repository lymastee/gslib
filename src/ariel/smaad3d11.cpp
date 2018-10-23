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

#include <gslib/error.h>
#include <ariel/smaa.h>
#include <ariel/rendersysd3d11.h>

#include <smaa/areatex.h>
#include <smaa/searchtex.h>

#include "ariel_smaa_edge_detection_vs.h"
#include "ariel_smaa_blending_weight_calculation_vs.h"
#include "ariel_smaa_neighborhood_blending_vs.h"
#include "ariel_smaa_luma_edge_detection_ps.h"
#include "ariel_smaa_color_edge_detection_ps.h"
#include "ariel_smaa_blending_weight_calculation_ps.h"
#include "ariel_smaa_neighborhood_blending_ps.h"

__ariel_begin__

template<class c>
static void release_any(c& cptr)
{
    if(cptr) {
        cptr->Release();
        cptr = nullptr;
    }
}

static DXGI_FORMAT smaa_make_typeless(DXGI_FORMAT fmt)
{
    switch(fmt)
    {
    case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
    case DXGI_FORMAT_R8G8B8A8_UNORM:
    case DXGI_FORMAT_R8G8B8A8_UINT:
    case DXGI_FORMAT_R8G8B8A8_SNORM:
    case DXGI_FORMAT_R8G8B8A8_SINT:
        return DXGI_FORMAT_R8G8B8A8_TYPELESS;
    case DXGI_FORMAT_BC1_UNORM_SRGB:
    case DXGI_FORMAT_BC1_UNORM:
        return DXGI_FORMAT_BC1_TYPELESS;
    case DXGI_FORMAT_BC2_UNORM_SRGB:
    case DXGI_FORMAT_BC2_UNORM:
        return DXGI_FORMAT_BC2_TYPELESS;
    case DXGI_FORMAT_BC3_UNORM_SRGB:
    case DXGI_FORMAT_BC3_UNORM:
        return DXGI_FORMAT_BC3_TYPELESS;
    }
    return fmt;
}

static void smaa_create_views(ID3D11Device* dev, DXGI_FORMAT format, ID3D11Texture2D* tex, ID3D11ShaderResourceView*& srv, ID3D11RenderTargetView*& rtv)
{
    assert(dev && tex);
    D3D11_RENDER_TARGET_VIEW_DESC rtdesc;
    rtdesc.Format = format;
    rtdesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    rtdesc.Texture2D.MipSlice = 0;
    dev->CreateRenderTargetView(tex, &rtdesc, &rtv);
    D3D11_SHADER_RESOURCE_VIEW_DESC srdesc;
    srdesc.Format = format;
    srdesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srdesc.Texture2D.MostDetailedMip = 0;
    srdesc.Texture2D.MipLevels = 1;
    dev->CreateShaderResourceView(tex, &srdesc, &srv);
}

smaa_render_target::smaa_render_target(rendersys* rsys, int w, int h, uint format)
{
    assert(rsys);
    auto* rsysd3d = static_cast<rendersys_d3d11*>(rsys);
    auto* dev = rsysd3d->get_device();
    assert(dev);
    _width = w;
    _height = h;
    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Width = w;
    desc.Height = h;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = smaa_make_typeless((DXGI_FORMAT)format);
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    dev->CreateTexture2D(&desc, nullptr, &_tex);
    assert(_tex);
    smaa_create_views(dev, (DXGI_FORMAT)format, _tex, _srv, _rtv);
    assert(_srv && _rtv);
}

smaa_render_target::~smaa_render_target()
{
    release_any(_tex);
    release_any(_rtv);
    release_any(_srv);
}

void smaa::initialize(rendersys* rsys)
{
    assert(rsys);
    rendersys::vertex_format_desc desc[] =
    {
        { "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D10_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D10_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    rendersys::create_shader_context ctx;
    rsys->begin_create_shader(ctx, g_ariel_smaa_edge_detection_vs, sizeof(g_ariel_smaa_edge_detection_vs));
    _edge_detection_vs = rsys->create_vertex_shader(ctx);
    assert(_edge_detection_vs);
    _fullscreen_vf = rsys->create_vertex_format(ctx, desc, _countof(desc));
    assert(_fullscreen_vf);
    rsys->end_create_shader(ctx);
    rsys->begin_create_shader(ctx, g_ariel_smaa_blending_weight_calculation_vs, sizeof(g_ariel_smaa_blending_weight_calculation_vs));
    _blending_weight_vs = rsys->create_vertex_shader(ctx);
    assert(_blending_weight_vs);
    rsys->end_create_shader(ctx);
    rsys->begin_create_shader(ctx, g_ariel_smaa_neighborhood_blending_vs, sizeof(g_ariel_smaa_neighborhood_blending_vs));
    _neighbor_blending_vs = rsys->create_vertex_shader(ctx);
    assert(_neighbor_blending_vs);
    rsys->end_create_shader(ctx);
    rsys->begin_create_shader(ctx, g_ariel_smaa_luma_edge_detection_ps, sizeof(g_ariel_smaa_luma_edge_detection_ps));
    _luma_edge_detection_ps = rsys->create_pixel_shader(ctx);
    assert(_luma_edge_detection_ps);
    rsys->end_create_shader(ctx);
    rsys->begin_create_shader(ctx, g_ariel_smaa_color_edge_detection_ps, sizeof(g_ariel_smaa_color_edge_detection_ps));
    _color_edge_detection_ps = rsys->create_pixel_shader(ctx);
    assert(_color_edge_detection_ps);
    rsys->end_create_shader(ctx);
    rsys->begin_create_shader(ctx, g_ariel_smaa_blending_weight_calculation_ps, sizeof(g_ariel_smaa_blending_weight_calculation_ps));
    _blending_weight_ps = rsys->create_pixel_shader(ctx);
    assert(_blending_weight_ps);
    rsys->end_create_shader(ctx);
    rsys->begin_create_shader(ctx, g_ariel_smaa_neighborhood_blending_ps, sizeof(g_ariel_smaa_neighborhood_blending_ps));
    _neighbor_blending_ps = rsys->create_pixel_shader(ctx);
    assert(_neighbor_blending_ps);
    rsys->end_create_shader(ctx);
    _cb_configs = rsys->create_constant_buffer(pack_cb_size<smaa_configs>(), false, true);
    assert(_cb_configs);
    _cb_index = 0;
    initialize_fullscreen_triangle(rsys);
    load_area_tex(rsys);
    load_search_tex(rsys);
}

void smaa::initialize_fullscreen_triangle(rendersys* rsys)
{
    assert(rsys);
    struct
    {
        vec3    position;
        vec2    texcoord;
    } vertices[3];
    vertices[0].position = vec3(-1.0f, -1.0f, 1.0f);
    vertices[1].position = vec3(-1.0f, 3.0f, 1.0f);
    vertices[2].position = vec3(3.0f, -1.0f, 1.0f);
    vertices[0].texcoord = vec2(0.0f, 1.0f);
    vertices[1].texcoord = vec2(0.0f, -1.0f);
    vertices[2].texcoord = vec2(2.0f, 1.0f);
    _fullscreen_triangle = rsys->create_vertex_buffer(sizeof(vertices[0]), 3, false, false, D3D11_USAGE_IMMUTABLE, vertices);
    assert(_fullscreen_triangle);
}

void smaa::destroy_miscs()
{
    release_any(_fullscreen_triangle);
    release_any(_cb_configs);
    release_any(_fullscreen_vf);
    release_any(_edge_detection_vs);
    release_any(_blending_weight_vs);
    release_any(_neighbor_blending_vs);
    release_any(_luma_edge_detection_ps);
    release_any(_color_edge_detection_ps);
    release_any(_blending_weight_ps);
    release_any(_neighbor_blending_ps);
    release_any(_area_tex);
    release_any(_search_tex);
}

void smaa::prepare_render_targets(rendersys* rsys, int w, int h)
{
    assert(rsys);
    const auto& info = rsys->get_device_info();
    bool is_nvidia_card = (info.vendor_id == 0x10de);
    DXGI_FORMAT fmt = is_nvidia_card ? DXGI_FORMAT_R8G8B8A8_UNORM : DXGI_FORMAT_R8G8_UNORM;
    _edges_rt = new smaa_render_target(rsys, w, h, fmt);
    assert(_edges_rt);
    _blend_rt = new smaa_render_target(rsys, w, h, DXGI_FORMAT_R8G8B8A8_UNORM);
    assert(_blend_rt);
}

void smaa::load_area_tex(rendersys* rsys)
{
    assert(rsys);
    _area_tex = nullptr;
    _area_srv = nullptr;
    rendersys_d3d11* rsysd3d = static_cast<rendersys_d3d11*>(rsys);
    ID3D11Device* dev = rsysd3d->get_device();
    assert(dev);
    D3D11_SUBRESOURCE_DATA data;
    data.pSysMem = areaTexBytes;
    data.SysMemPitch = AREATEX_PITCH;
    data.SysMemSlicePitch = 0;
    D3D11_TEXTURE2D_DESC desc;
    desc.Width = AREATEX_WIDTH;
    desc.Height = AREATEX_HEIGHT;
    desc.MipLevels = desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_IMMUTABLE;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;
    if(FAILED(dev->CreateTexture2D(&desc, &data, &_area_tex))) {
        set_error(_t("create area tex failed."));
        return;
    }
    assert(_area_tex);
    D3D11_SHADER_RESOURCE_VIEW_DESC srvdesc;
    srvdesc.Format = desc.Format;
    srvdesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvdesc.Texture2D.MostDetailedMip = 0;
    srvdesc.Texture2D.MipLevels = 1;
    if(FAILED(dev->CreateShaderResourceView(_area_tex, &srvdesc, &_area_srv))) {
        set_error(_t("create SRV for area tex failed."));
        return;
    }
    assert(_area_srv);
}

void smaa::load_search_tex(rendersys* rsys)
{
    assert(rsys);
    _search_tex = nullptr;
    _search_srv = nullptr;
    rendersys_d3d11* rsysd3d = static_cast<rendersys_d3d11*>(rsys);
    ID3D11Device* dev = rsysd3d->get_device();
    assert(dev);
    D3D11_SUBRESOURCE_DATA data;
    data.pSysMem = searchTexBytes;
    data.SysMemPitch = SEARCHTEX_PITCH;
    data.SysMemSlicePitch = 0;
    D3D11_TEXTURE2D_DESC desc;
    desc.Width = SEARCHTEX_WIDTH;
    desc.Height = SEARCHTEX_HEIGHT;
    desc.MipLevels = desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_IMMUTABLE;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;
    if(FAILED(dev->CreateTexture2D(&desc, &data, &_search_tex))) {
        set_error(_t("create search tex failed."));
        return;
    }
    assert(_search_tex);
    D3D11_SHADER_RESOURCE_VIEW_DESC srvdesc;
    srvdesc.Format = desc.Format;
    srvdesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvdesc.Texture2D.MostDetailedMip = 0;
    srvdesc.Texture2D.MipLevels = 1;
    if(FAILED(dev->CreateShaderResourceView(_search_tex, &srvdesc, &_search_srv))) {
        set_error(_t("create SRV for search tex failed."));
        return;
    }
    assert(_search_srv);
}

__ariel_end__
