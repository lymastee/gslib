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

#include <ariel/config.h>
#include <ariel/texbatch.h>

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
    int left = round(rc.left);
    int top = round(rc.top);
    int width = src.get_width();
    int height = src.get_height();
    int right = left + width;
    int bottom = top + height;
    int width_in_bytes = width * sizeof(pink::color);       /* unsafe */
    for(int i = top; i < bottom; i ++) {
        auto* destptr = img.get_data(left, i);
        auto* srcptr = src.get_data(0, i - top);
        memcpy(destptr, srcptr, width_in_bytes);
    }
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
        auto* srcptr = (pink::color*)src.get_data(0, i - left);
        for(int j = top; j < bottom; j ++)
            *(pink::color*)img.get_data(i, j) = srcptr[j - top];
    }
}

static void write_image_source(image& img, const image& src, const rectf& rc)
{
    bool transposed = is_image_transposed(src, rc);
    if(is_image_transposed(src, rc))
        return write_image_transposed(img, src, rc);
    write_image_non_transposed(img, src, rc);
}

tex_batcher::tex_batcher()
{
    _gap = 1.f;
}

void tex_batcher::add_image(image* p)
{
    assert(p);
    _location_map.try_emplace(p, rectf());
}

void tex_batcher::arrange()
{
    if(_location_map.empty())
        return;
    rp_input_list inputs;
    prepare_input_list(inputs);
    _rect_packer.pack_automatically(inputs);
    _rect_packer.for_each([this](void* binding, const rp_rect& rc, bool transposed) {
        auto* img = reinterpret_cast<image*>(binding);
        auto f = _location_map.find(img);
        assert(f != _location_map.end());
        float w = rc.width - _gap;
        float h = rc.height - _gap;
        if(transposed)
            gs_swap(w, h);
        assert(w == img->get_width() && h == img->get_height());
        f->second.set_rect(rc.left() + _gap, rc.top() + _gap, w, h);
    });
}

render_texture2d* tex_batcher::create_texture(rendersys* rsys) const
{
    assert(rsys);
    float w = get_width(), h = get_height();
    /* prepare image */
    image img;
    img.create(image::fmt_rgba, (int)ceil(w), (int)ceil(h));
    img.init(pink::color(255, 255, 255, 0));
    for(const auto& value : _location_map)
        write_image_source(img, *value.first, value.second);
#if use_rendersys_d3d_11
    render_texture2d* p = rsys->create_texture2d(img, 1, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0);
#endif
    assert(p);
    return p;
}

void tex_batcher::prepare_input_list(rp_input_list& inputs)
{
    for(auto& p : _location_map) {
        auto* img = p.first;
        assert(img);
        rp_input input;
        input.width = img->get_width() + _gap;
        input.height = img->get_height() + _gap;
        input.binding = img;
        inputs.push_back(input);
    }
}

__ariel_end__
