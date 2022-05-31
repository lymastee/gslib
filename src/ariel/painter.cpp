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

#include <assert.h>
#include <gslib/error.h>
#include <gslib/utility.h>
#include <ariel/painter.h>
#include <ariel/scene.h>
#include <ariel/textureop.h>

__ariel_begin__

painter_picture_data::painter_picture_data(texture2d* p)
{
    assert(p);
    _image = p;
    int w = 0, h = 0;
    textureop::get_texture_dimension(p, w, h);
    _src_rect.set_rect(0.f, 0.f, (float)w, (float)h);
}

painter_picture_data::painter_picture_data(texture2d* p, const rectf& rc)
{
    assert(p);
    _image = p;
    _src_rect = rc;
}

painter::~painter()
{
    destroy_text_image_cache();
}

void painter::on_draw_begin()
{
    destroy_text_image_cache();
}

void painter::on_draw_end()
{
}

void painter::set_hints(uint hints, bool enable)
{
    if(enable)
        _hints |= hints;
    else
        _hints &= ~hints;
}

void painter::set_font(const font& ft)
{
    scene* scn = scene::get_singleton_ptr();
    assert(scn);
    fontsys* fsys = scn->get_fontsys();
    assert(fsys);
    fsys->set_font(ft);
}

void painter::save()
{
    _ctxst.push_back(_context);
    _context.set_transform(mat3().identity());
}

void painter::restore()
{
    if(_ctxst.empty())
        return;
    _context = _ctxst.back();
    _ctxst.pop_back();
}

void painter::get_text_dimension(const gchar* str, int& w, int& h, int len)
{
    assert(str);
    scene* scn = scene::get_singleton_ptr();
    assert(scn);
    fontsys* fsys = scn->get_fontsys();
    assert(fsys);
    fsys->query_size(str, w, h, len);
}

void painter::draw_image(texture2d* img, float x, float y)
{
    assert(img);
    save();
    int w = 0, h = 0;
    textureop::get_texture_dimension(img, w, h);
    painter_brush brush;
    painter_extra_data ext;
    ext.reset(new painter_picture_data(img, rectf(0.f, 0.f, (float)w, (float)h)), [](painter_picture_data* p) { delete p; });
    brush.set_tag(painter_brush::picture);
    brush.set_extra(ext);
    set_brush(brush);
    draw_rect(rectf(x, y, (float)w, (float)h));
    restore();
}

void painter::draw_image(texture2d* img, const rectf& dest, const rectf& src)
{
    assert(img);
#if defined(DEBUG) || (_DEBUG)
    int w = 0, h = 0;
    textureop::get_texture_dimension(img, w, h);
    assert(src.left < w && src.top < h && src.right < w && src.bottom < h);
#endif
    save();
    painter_brush brush;
    painter_extra_data ext;
    ext.reset(new painter_picture_data(img, src), [](painter_picture_data* p) { delete p; });
    brush.set_tag(painter_brush::picture);
    brush.set_extra(ext);
    set_brush(brush);
    draw_rect(dest);
    restore();
}

void painter::draw_line(const vec2& p1, const vec2& p2, const color& cr)
{
    save();
    painter_pen pen;
    pen.set_tag(painter_pen::solid);
    pen.set_color(cr);
    set_pen(pen);
    draw_line(p1, p2);
    restore();
}

void painter::draw_rect(const rectf& rc, const color& cr)
{
    save();
    painter_brush brush;
    brush.set_tag(painter_brush::solid);
    brush.set_color(cr);
    set_brush(brush);
    draw_rect(rc);
    restore();
}

void painter::draw_text(const gchar* str, float x, float y, const color& cr, int length)
{
    if(!str || !length)
        return;
    scene* scn = scene::get_singleton_ptr();
    assert(scn);
    fontsys* fsys = scn->get_fontsys();
    assert(fsys);
    com_ptr<texture2d> tex;
    int margin = 1;
    if(!fsys->create_text_texture(&tex, str, margin, cr, length)) {
        assert(!"create text texture failed.");
        return;
    }
    assert(tex);
    _text_image_cache.push_back(tex.get());
    int w = 0, h = 0;
    textureop::get_texture_dimension(tex.get(), w, h);
    float cw = (float)w - 2.f * margin;
    float ch = (float)h - 2.f * margin;
    assert(cw >= 0.f && ch >= 0.f);
    draw_image(tex.detach(), rectf(x, y, cw, ch), rectf((float)margin, (float)margin, cw, ch));
}

void painter::get_transform_recursively(mat3& m) const
{
    m = _context.get_trasnform();
    if(_ctxst.empty())
        return;
    for(auto i = std::prev(_ctxst.end());; -- i) {
        m *= i->get_trasnform();
        if(i == _ctxst.begin())
            break;
    }
}

void painter::destroy_text_image_cache()
{
    for(auto* img : _text_image_cache) {
        assert(img);
        release_texture2d(img);
    }
    _text_image_cache.clear();
}

__ariel_end__
