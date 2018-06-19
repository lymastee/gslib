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

#include <ariel/textureop.h>
#include <ariel/rendersysd3d11.h>

#include "ariel_transpose_image_cs.h"
#include "ariel_initialize_image_cs.h"
#include "ariel_set_brightness_cs.h"
#include "ariel_set_gray_cs.h"
#include "ariel_set_fade_cs.h"
#include "ariel_set_inverse_cs.h"

__ariel_begin__

void textureop::copy_texture_rect(render_texture2d* dest, render_texture2d* src, const rectf& rc)
{
    assert(dest && src);
    if(!check_valid_device(dest) || !check_valid_device(src))
        return;
    auto* dc = get_immediate_context();
    assert(dc);
    int w, h;
    get_texture_dimension(src, w, h);
    w = gs_min(w, (int)ceil(rc.width()));
    h = gs_min(h, (int)ceil(rc.height()));
    D3D11_BOX box;
    box.left = 0;
    box.top = 0;
    box.right = w;
    box.bottom = h;
    box.front = 0;
    box.back = 1;
    dc->CopySubresourceRegion(dest, 0, (uint)floor(rc.left), (uint)floor(rc.top), 0, src, 0, &box);
}

void textureop::initialize_texture_rect(unordered_access_view* dest, const color& cr, const rectf& rc)
{
    assert(dest);
    if(!check_valid_device(dest))
        return;
    /* shader instance */
    static com_ptr<compute_shader> spcs;
    if(!spcs) {
        rendersys::create_shader_context ctx;
        _rsys->begin_create_shader(ctx, g_ariel_initialize_image_cs, sizeof(g_ariel_initialize_image_cs));
        auto* cs = _rsys->create_compute_shader(ctx);
        assert(cs);
        _rsys->end_create_shader(ctx);
        spcs.attach(cs);
    }
    if(!check_valid_device(spcs.get()))
        return;
    /* constant buffer instance */
    auto* cb = get_constant_buffer();
    if(!cb)
        return;
    /* setup position */
    com_ptr<ID3D11Resource> spres;
    dest->GetResource(&spres);
    assert(spres);
    auto* tex = static_cast<ID3D11Texture2D*>(spres.get());
    int w, h;
    get_texture_dimension(tex, w, h);
    w = gs_min(w, (int)ceil(rc.right)) - (int)floor(rc.left);
    h = gs_min(h, (int)ceil(rc.bottom)) - (int)floor(rc.top);
    cb_configs cfg;
    cfg.x = (int)floor(rc.left);
    cfg.y = (int)floor(rc.top);
    cfg.cr = vec4((float)cr.red / 255.f, (float)cr.green / 255.f, (float)cr.blue / 255.f, (float)cr.alpha / 255.f);
    /* do initialize */
    auto* dc = get_immediate_context();
    assert(dc);
    dc->CSSetShader(spcs.get(), nullptr, 0);
    dc->CSSetUnorderedAccessViews(0, 1, &dest, nullptr);
    _rsys->update_buffer(cb, sizeof(cb_configs), &cfg);
    dc->CSSetConstantBuffers(0, 1, &cb);
    dc->Dispatch(w, h, 1);
    dc->CSSetShader(nullptr, nullptr, 0);
    unordered_access_view* uav = nullptr;
    dc->CSSetUnorderedAccessViews(0, 1, &uav, nullptr);
    cb = nullptr;
    dc->CSSetConstantBuffers(0, 1, &cb);
}

void textureop::transpose_texture_rect(unordered_access_view* dest, render_texture2d* src, const rectf& rc)
{
    assert(dest && src);
    if(!check_valid_device(dest) || !check_valid_device(src))
        return;
    /* shader instance */
    static com_ptr<compute_shader> spcs;
    if(!spcs) {
        rendersys::create_shader_context ctx;
        _rsys->begin_create_shader(ctx, g_ariel_transpose_image_cs, sizeof(g_ariel_transpose_image_cs));
        auto* cs = _rsys->create_compute_shader(ctx);
        assert(cs);
        _rsys->end_create_shader(ctx);
        spcs.attach(cs);
    }
    if(!check_valid_device(spcs.get()))
        return;
    /* constant buffer instance */
    auto* cb = get_constant_buffer();
    if(!cb)
        return;
    /* create srv for source */
    com_ptr<shader_resource_view> spsrv;
    auto* srv = _rsys->create_shader_resource_view(src);
    assert(srv);
    spsrv.attach(srv);
    /* setup transpose position */
    int w, h;
    get_texture_dimension(src, w, h);
    w = gs_min(w, (int)ceil(rc.height()));
    h = gs_min(h, (int)ceil(rc.width()));
    cb_configs cfg;
    cfg.x = (int)floor(rc.left);
    cfg.y = (int)floor(rc.top);
    /* do transpose */
    auto* dc = get_immediate_context();
    assert(dc);
    dc->CSSetShader(spcs.get(), nullptr, 0);
    dc->CSSetShaderResources(0, 1, &srv);
    dc->CSSetUnorderedAccessViews(0, 1, &dest, nullptr);
    _rsys->update_buffer(cb, sizeof(cb_configs), &cfg);
    dc->CSSetConstantBuffers(0, 1, &cb);
    dc->Dispatch(w, h, 1);
    dc->CSSetShader(nullptr, nullptr, 0);
    srv = nullptr;
    dc->CSSetShaderResources(0, 1, &srv);
    unordered_access_view* uav = nullptr;
    dc->CSSetUnorderedAccessViews(0, 1, &uav, nullptr);
    cb = nullptr;
    dc->CSSetConstantBuffers(0, 1, &cb);
}

void textureop::set_brightness(unordered_access_view* dest, render_texture2d* src, float s)
{
    assert(dest && src);
    if(!check_valid_device(dest) || !check_valid_device(src))
        return;
    /* shader instance */
    static com_ptr<compute_shader> spcs;
    if(!spcs) {
        rendersys::create_shader_context ctx;
        _rsys->begin_create_shader(ctx, g_ariel_set_brightness_cs, sizeof(g_ariel_set_brightness_cs));
        auto* cs = _rsys->create_compute_shader(ctx);
        assert(cs);
        _rsys->end_create_shader(ctx);
        spcs.attach(cs);
    }
    if(!check_valid_device(spcs.get()))
        return;
    /* constant buffer instance */
    auto* cb = get_constant_buffer();
    if(!cb)
        return;
    /* create srv for source */
    com_ptr<shader_resource_view> spsrv;
    auto* srv = _rsys->create_shader_resource_view(src);
    assert(srv);
    spsrv.attach(srv);
    /* setup position */
    int w, h;
    get_texture_dimension(src, w, h);
    cb_configs cfg;
    cfg.arg = s;
    /* proc */
    auto* dc = get_immediate_context();
    assert(dc);
    dc->CSSetShader(spcs.get(), nullptr, 0);
    dc->CSSetShaderResources(0, 1, &srv);
    dc->CSSetUnorderedAccessViews(0, 1, &dest, nullptr);
    _rsys->update_buffer(cb, sizeof(cb_configs), &cfg);
    dc->CSSetConstantBuffers(0, 1, &cb);
    dc->Dispatch(w, h, 1);
    dc->CSSetShader(nullptr, nullptr, 0);
    srv = nullptr;
    dc->CSSetShaderResources(0, 1, &srv);
    unordered_access_view* uav = nullptr;
    dc->CSSetUnorderedAccessViews(0, 1, &uav, nullptr);
    cb = nullptr;
    dc->CSSetConstantBuffers(0, 1, &cb);
}

void textureop::set_fade(unordered_access_view* dest, render_texture2d* src, float s)
{
    assert(dest && src);
    if(!check_valid_device(dest) || !check_valid_device(src))
        return;
    /* shader instance */
    static com_ptr<compute_shader> spcs;
    if(!spcs) {
        rendersys::create_shader_context ctx;
        _rsys->begin_create_shader(ctx, g_ariel_set_fade_cs, sizeof(g_ariel_set_fade_cs));
        auto* cs = _rsys->create_compute_shader(ctx);
        assert(cs);
        _rsys->end_create_shader(ctx);
        spcs.attach(cs);
    }
    if(!check_valid_device(spcs.get()))
        return;
    /* constant buffer instance */
    auto* cb = get_constant_buffer();
    if(!cb)
        return;
    /* create srv for source */
    com_ptr<shader_resource_view> spsrv;
    auto* srv = _rsys->create_shader_resource_view(src);
    assert(srv);
    spsrv.attach(srv);
    /* setup position */
    int w, h;
    get_texture_dimension(src, w, h);
    cb_configs cfg;
    cfg.arg = s;
    /* proc */
    auto* dc = get_immediate_context();
    assert(dc);
    dc->CSSetShader(spcs.get(), nullptr, 0);
    dc->CSSetShaderResources(0, 1, &srv);
    dc->CSSetUnorderedAccessViews(0, 1, &dest, nullptr);
    _rsys->update_buffer(cb, sizeof(cb_configs), &cfg);
    dc->CSSetConstantBuffers(0, 1, &cb);
    dc->Dispatch(w, h, 1);
    dc->CSSetShader(nullptr, nullptr, 0);
    srv = nullptr;
    dc->CSSetShaderResources(0, 1, &srv);
    unordered_access_view* uav = nullptr;
    dc->CSSetUnorderedAccessViews(0, 1, &uav, nullptr);
    cb = nullptr;
    dc->CSSetConstantBuffers(0, 1, &cb);
}

void textureop::set_gray(unordered_access_view* dest, render_texture2d* src)
{
    assert(dest && src);
    if(!check_valid_device(dest) || !check_valid_device(src))
        return;
    /* shader instance */
    static com_ptr<compute_shader> spcs;
    if(!spcs) {
        rendersys::create_shader_context ctx;
        _rsys->begin_create_shader(ctx, g_ariel_set_gray_cs, sizeof(g_ariel_set_gray_cs));
        auto* cs = _rsys->create_compute_shader(ctx);
        assert(cs);
        _rsys->end_create_shader(ctx);
        spcs.attach(cs);
    }
    if(!check_valid_device(spcs.get()))
        return;
    /* create srv for source */
    com_ptr<shader_resource_view> spsrv;
    auto* srv = _rsys->create_shader_resource_view(src);
    assert(srv);
    spsrv.attach(srv);
    /* setup position */
    int w, h;
    get_texture_dimension(src, w, h);
    /* proc */
    auto* dc = get_immediate_context();
    assert(dc);
    dc->CSSetShader(spcs.get(), nullptr, 0);
    dc->CSSetShaderResources(0, 1, &srv);
    dc->CSSetUnorderedAccessViews(0, 1, &dest, nullptr);
    dc->Dispatch(w, h, 1);
    dc->CSSetShader(nullptr, nullptr, 0);
    srv = nullptr;
    dc->CSSetShaderResources(0, 1, &srv);
    unordered_access_view* uav = nullptr;
    dc->CSSetUnorderedAccessViews(0, 1, &uav, nullptr);
}

void textureop::set_inverse(unordered_access_view* dest, render_texture2d* src)
{
    assert(dest && src);
    if(!check_valid_device(dest) || !check_valid_device(src))
        return;
    /* shader instance */
    static com_ptr<compute_shader> spcs;
    if(!spcs) {
        rendersys::create_shader_context ctx;
        _rsys->begin_create_shader(ctx, g_ariel_set_inverse_cs, sizeof(g_ariel_set_inverse_cs));
        auto* cs = _rsys->create_compute_shader(ctx);
        assert(cs);
        _rsys->end_create_shader(ctx);
        spcs.attach(cs);
    }
    if(!check_valid_device(spcs.get()))
        return;
    /* create srv for source */
    com_ptr<shader_resource_view> spsrv;
    auto* srv = _rsys->create_shader_resource_view(src);
    assert(srv);
    spsrv.attach(srv);
    /* setup position */
    int w, h;
    get_texture_dimension(src, w, h);
    /* proc */
    auto* dc = get_immediate_context();
    assert(dc);
    dc->CSSetShader(spcs.get(), nullptr, 0);
    dc->CSSetShaderResources(0, 1, &srv);
    dc->CSSetUnorderedAccessViews(0, 1, &dest, nullptr);
    dc->Dispatch(w, h, 1);
    dc->CSSetShader(nullptr, nullptr, 0);
    srv = nullptr;
    dc->CSSetShaderResources(0, 1, &srv);
    unordered_access_view* uav = nullptr;
    dc->CSSetUnorderedAccessViews(0, 1, &uav, nullptr);
}

void textureop::initialize_texture_rect(render_texture2d* dest, const color& cr, const rectf& rc)
{
    assert(dest);
    if(!check_valid_device(dest))
        return;
    com_ptr<unordered_access_view> spuav;
    spuav.attach(_rsys->create_unordered_access_view(dest));
    assert(spuav);
    initialize_texture_rect(spuav.get(), cr, rc);
}

void textureop::transpose_texture_rect(render_texture2d* dest, render_texture2d* src, const rectf& rc)
{
    assert(dest && src);
    if(!check_valid_device(dest))
        return;
    com_ptr<unordered_access_view> spuav;
    spuav.attach(_rsys->create_unordered_access_view(dest));
    assert(spuav);
    transpose_texture_rect(spuav.get(), src, rc);
}

void textureop::set_brightness(render_texture2d* dest, render_texture2d* src, float s)
{
    assert(dest && src);
    if(!check_valid_device(dest))
        return;
    com_ptr<unordered_access_view> spuav;
    spuav.attach(_rsys->create_unordered_access_view(dest));
    assert(spuav);
    set_brightness(spuav.get(), src, s);
}

void textureop::set_gray(render_texture2d* dest, render_texture2d* src)
{
    assert(dest && src);
    if(!check_valid_device(dest))
        return;
    com_ptr<unordered_access_view> spuav;
    spuav.attach(_rsys->create_unordered_access_view(dest));
    assert(spuav);
    set_gray(spuav.get(), src);
}

void textureop::set_fade(render_texture2d* dest, render_texture2d* src, float s)
{
    assert(dest && src);
    if(!check_valid_device(dest))
        return;
    com_ptr<unordered_access_view> spuav;
    spuav.attach(_rsys->create_unordered_access_view(dest));
    assert(spuav);
    set_fade(spuav.get(), src, s);
}

void textureop::set_inverse(render_texture2d* dest, render_texture2d* src)
{
    assert(dest && src);
    if(!check_valid_device(dest))
        return;
    com_ptr<unordered_access_view> spuav;
    spuav.attach(_rsys->create_unordered_access_view(dest));
    assert(spuav);
    set_inverse(spuav.get(), src);
}

void textureop::get_texture_dimension(render_texture2d* p, int& w, int& h)
{
    assert(p);
    D3D11_TEXTURE2D_DESC desc;
    p->GetDesc(&desc);
    w = desc.Width;
    h = desc.Height;
}

void textureop::get_assoc_device(render_texture2d* p, render_device** ppdev)
{
    assert(p && ppdev);
    assert(!*ppdev);
    p->GetDevice(ppdev);
}

bool textureop::convert_to_image(image& img, render_texture2d* tex)
{
    assert(tex);
    com_ptr<render_device> spdev;
    tex->GetDevice(&spdev);
    assert(spdev);
    com_ptr<render_context> spdc;
    spdev->GetImmediateContext(&spdc);
    assert(spdc);
    D3D11_TEXTURE2D_DESC desc;
    tex->GetDesc(&desc);
    desc.Usage = D3D11_USAGE_STAGING;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    desc.BindFlags = 0;
    desc.MiscFlags = 0;
    com_ptr<ID3D11Texture2D> cpytex;
    if(FAILED(spdev->CreateTexture2D(&desc, nullptr, &cpytex))) {
        assert(!"create staging texture failed.");
        return false;
    }
    spdc->CopyResource(cpytex.get(), tex);
    img.create(image::fmt_rgba, desc.Width, desc.Height);
    img.enable_alpha_channel(true);
    D3D11_MAPPED_SUBRESOURCE subres;
    spdc->Map(cpytex.get(), 0, D3D11_MAP_READ, 0, &subres);
    byte* src = (byte*)subres.pData;
    byte* bits = img.get_data(0, 0);
    int bpl = img.get_bytes_per_line();
    for(uint i = 0; i < desc.Height; i ++)
        memcpy(bits + (i * bpl), src + (i * subres.RowPitch), bpl);
    spdc->Unmap(cpytex.get(), 0);
    return true;
}

render_device* textureop::get_device() const
{
    assert(_rsys);
    return static_cast<rendersys_d3d11*>(_rsys)->get_device();
}

render_context* textureop::get_immediate_context() const
{
    assert(_rsys);
    return static_cast<rendersys_d3d11*>(_rsys)->get_immediate_context();
}

render_constant_buffer* textureop::get_constant_buffer() const
{
    static com_ptr<render_constant_buffer> spcb;
    if(!spcb) {
        auto* cb = _rsys->create_constant_buffer(pack_cb_size<cb_configs>(), false, true);
        assert(cb);
        spcb.attach(cb);
    }
    if(!check_valid_device(spcb.get()))
        return nullptr;
    return spcb.get();
}

template<class _cls>
bool textureop::check_valid_device(_cls* p) const
{
    assert(p);
    com_ptr<render_device> spdc;
    p->GetDevice(&spdc);
    if(spdc.get() != get_device()) {
        assert(!"different device, cannot proceed operation.");
        return false;
    }
    return true;
}

__ariel_end__
