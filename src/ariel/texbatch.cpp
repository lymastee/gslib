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

#include <gslib/std.h>
#include <ariel/config.h>
#include <ariel/texbatch.h>
#include <ariel/textureop.h>

__ariel_begin__

static bool is_image_transposed(const image& img, const rectf& rc)
{
    assert((img.get_width() == rc.width() && img.get_height() == rc.height()) ||
        (img.get_width() == rc.height() && img.get_height() == rc.width())
        );
    return !(img.get_width() == rc.width() && img.get_height() == rc.height());
}

static void write_image_non_transposed(image& img, const image& src, const rectf& rc)
{
    assert(src.get_width() == rc.width() && src.get_height() == rc.height());
    img.copy(src, round(rc.left), round(rc.top), round(rc.width()), round(rc.height()), 0, 0);
}

static void write_image_transposed(image& img, const image& src, const rectf& rc)
{
    assert(src.get_width() == rc.height() && src.get_height() == rc.width());
    int left = round(rc.left);
    int top = round(rc.top);
    int width = src.get_height();
    int height = src.get_width();
    int right = left + width;
    int bottom = top + height;
    for(int i = left; i < right; i ++) {
        auto* srcptr = (color*)src.get_data(0, i - left);
        for(int j = top; j < bottom; j ++)
            *(color*)img.get_data(i, j) = srcptr[j - top];
    }
}

static void write_image_source(image& img, const image& src, const rectf& rc)
{
    is_image_transposed(src, rc) ?
        write_image_transposed(img, src, rc) :
        write_image_non_transposed(img, src, rc);
}

static bool is_texture_transposed(texture2d* tex, const rectf& rc)
{
    assert(tex);
    int width, height;
    textureop::get_texture_dimension(tex, width, height);
    assert((width == rc.width() && height == rc.height()) ||
        (width == rc.height() && height == rc.width())
    );
    return !(width == rc.width() && height == rc.height());
}

static void write_texture_source(rendersys* rsys, unordered_access_view* uav, texture2d* tex, texture2d* src, const rectf& rc)
{
    assert(rsys);
    textureop texop(rsys);
    is_texture_transposed(src, rc) ?
        texop.transpose_rect(uav, src, rc) :
        texop.copy_rect(tex, src, (int)rc.left, (int)rc.top);
}

tex_batcher::tex_batcher()
{
    _gap = 1.f;
}

void tex_batcher::add_image(const image* p)
{
#ifdef _GS_BATCH_IMAGE
    assert(p);
    _location_map.try_emplace(p, rectf());
#else
    assert(!"unsupported function.");
#endif
}

void tex_batcher::add_texture(texture2d* p)
{
#ifdef _GS_BATCH_TEXTURE
    assert(p);
    _location_map.try_emplace(p, rectf());
#else
    assert(!"unsupported function.");
#endif
}

void tex_batcher::arrange()
{
    if(_location_map.empty())
        return;
    rp_input_list inputs;
    prepare_input_list(inputs);
    _rect_packer.pack_automatically(inputs);
#if defined(_GS_BATCH_IMAGE)
    _rect_packer.for_each([this](void* binding, const rp_rect& rc, bool transposed) {
        auto* img = reinterpret_cast<image*>(binding);
        auto f = _location_map.find(img);
        assert(f != _location_map.end());
        float w = rc.width - _gap;
        float h = rc.height - _gap;
        assert((!transposed && w == img->get_width() && h == img->get_height()) ||
            (transposed && h == img->get_width() && w == img->get_height())
            );
        f->second.set_rect(rc.left() + _gap, rc.top() + _gap, w, h);
    });
#elif defined(_GS_BATCH_TEXTURE)
    _rect_packer.for_each([this](void* binding, const rp_rect& rc, bool transposed) {
        auto* tex = reinterpret_cast<texture2d*>(binding);
        auto f = _location_map.find(tex);
        assert(f != _location_map.end());
        float w = rc.width - _gap;
        float h = rc.height - _gap;
#if defined(_DEBUG) || defined(DEBUG)
        int width, height;
        textureop::get_texture_dimension(tex, width, height);
        assert((!transposed && w == width && h == height) ||
            (transposed && h == width && w == height)
        );
#endif
        f->second.set_rect(rc.left() + _gap, rc.top() + _gap, w, h);
    });
#endif
}

render_texture2d* tex_batcher::create_texture(rendersys* rsys) const
{
#if defined(_GS_BATCH_IMAGE)
    assert(rsys);
    image img;
    create_packed_image(img);
#if use_rendersys_d3d_11
    texture2d* p = rsys->create_texture2d(img, 1, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0);
#endif
    assert(p);
    return p;
#elif defined(_GS_BATCH_TEXTURE)
    float w = get_width(), h = get_height();
    texture2d* tex = rsys->create_texture2d((int)ceil(w), (int)ceil(h), DXGI_FORMAT_R8G8B8A8_UNORM, 1, D3D11_USAGE_DEFAULT, D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE, 0);
    assert(tex);
    com_ptr<unordered_access_view> spuav;
    auto* uav = rsys->create_unordered_access_view(tex);
    assert(uav);
    spuav.attach(uav);
    textureop(rsys).initialize_rect(uav, color(0, 0, 0, 0), rectf(0.f, 0.f, w, h));
    for(const auto& value : _location_map)
        write_texture_source(rsys, uav, tex, value.first, value.second);
    return tex;
#endif
}

void tex_batcher::create_packed_image(image& img) const
{
#if defined(_GS_BATCH_IMAGE)
    float w = get_width(), h = get_height();
    img.create(image::fmt_rgba, (int)ceil(w), (int)ceil(h));
    img.enable_alpha_channel(true);
    img.init(color(0, 0, 0, 0));
    for(const auto& value : _location_map)
        write_image_source(img, *value.first, value.second);
#elif defined(_GS_BATCH_TEXTURE)
    assert(!"unsupported, use create_texture() instead.");
#endif
}

void tex_batcher::tracing() const
{
    _rect_packer.tracing();
}

void tex_batcher::prepare_input_list(rp_input_list& inputs)
{
#if defined(_GS_BATCH_IMAGE)
    for(auto& p : _location_map) {
        auto* img = p.first;
        assert(img);
        rp_input input;
        input.width = img->get_width() + _gap;
        input.height = img->get_height() + _gap;
        input.binding = (void*)img;
        inputs.push_back(input);
    }
#elif defined(_GS_BATCH_TEXTURE)
    for(auto& p : _location_map) {
        auto* tex = p.first;
        assert(tex);
        int w, h;
        textureop::get_texture_dimension(tex, w, h);
        rp_input input;
        input.width = w + _gap;
        input.height = h + _gap;
        input.binding = (void*)tex;
        inputs.push_back(input);
    }
#endif
}

__ariel_end__
