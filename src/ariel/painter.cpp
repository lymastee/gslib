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

#include <assert.h>
#include <gslib/error.h>
#include <gslib/pool.h>
#include <ariel/painter.h>
#include <ariel/utility.h>
#include <ariel/scene.h>
#include <ariel/textureop.h>

__ariel_begin__

painter_linestrip::painter_linestrip()
{
    _closed = false;
    _tst_table = 0;
}

void painter_linestrip::swap(painter_linestrip& another)
{
    gs_swap(_closed, another._closed);
    _pts.swap(another._pts);
}

vec2* painter_linestrip::expand(int size)
{
    int oss = (int)_pts.size();
    _pts.resize(oss + size);
    return &_pts.at(oss);
}

void painter_linestrip::reverse()
{
    points pts;
    pts.assign(_pts.rbegin(), _pts.rend());
    _pts.swap(pts);
    if(is_clockwise_inited())
        _is_clock_wise = !_is_clock_wise;
}

void painter_linestrip::finish()
{
    if(_pts.front() == _pts.back()) {
        _pts.pop_back();
        set_closed(true);
    }
}

void painter_linestrip::transform(const mat3& m)
{
    for(vec2& v : _pts)
        v.transformcoord(v, m);
}

bool painter_linestrip::is_clockwise() const
{
    if(!_closed || _pts.size() <= 2)
        return false;
    if(is_clockwise_inited())
        return _is_clock_wise;
    set_clockwise_inited();
    float mx = -FLT_MAX;
    int mi = 0, cap = (int)_pts.size();
    for(int i = 0; i < cap; i ++) {
        if(_pts.at(i).x >= mx) {
            mx = _pts.at(i).x;
            mi = i;
        }
    }
    int prev = mi == 0 ? cap - 1 : mi - 1,
        post = mi == cap - 1 ? 0 : mi + 1;
    return _is_clock_wise = 
        !is_concave_angle(_pts.at(prev), _pts.at(mi), _pts.at(post));
}

bool painter_linestrip::is_convex() const
{
    assert(get_size() >= 3);
    if(is_convex_inited())
        return _is_convex;
    set_convex_inited();
    bool cw = is_clockwise();
    int cap = (int)_pts.size();
    for(int i = 2; i < cap; i ++) {
        if(is_concave_angle(_pts.at(i-2), _pts.at(i-1), _pts.at(i), cw))
            return _is_convex = false;
    }
    if(is_concave_angle(_pts.back(), _pts.front(), _pts.at(1), cw))
        return _is_convex = false;
    return _is_convex = 
        !is_concave_angle(_pts.at(cap-2), _pts.back(), _pts.front(), cw);
}

bool painter_linestrip::is_convex(int i) const
{
    bool cw = is_clockwise();
    int cap = (int)_pts.size();
    int prev = i == 0 ? cap - 1 : i - 1,
        post = i == cap - 1 ? 0 : i + 1;
    return !is_concave_angle(_pts.at(prev), _pts.at(i), _pts.at(post), cw);
}

void painter_linestrip::tracing() const
{
#ifdef _DEBUG
    if(_pts.empty())
        return;
    trace(_t("@!\n"));
    auto i = _pts.begin();
    trace(_t("@moveTo %f, %f;\n"), i->x, i->y);
    for(++ i; i != _pts.end(); ++ i) {
        trace(_t("@lineTo %f, %f;\n"), i->x, i->y);
    }
    i = _pts.begin();
    trace(_t("@lineTo %f, %f;\n"), i->x, i->y);
    trace(_t("@@\n"));
#endif
}

void painter_linestrip::tracing_segments() const
{
#ifdef _DEBUG
    if(_pts.empty())
        return;
    trace(_t("@!\n"));
    auto i = _pts.begin();
    trace(_t("@moveTo %f, %f;\n"), i->x, i->y);
    for(++ i; i != _pts.end(); ++ i) {
        trace(_t("@lineTo %f, %f;\n"), i->x, i->y);
        trace(_t("@moveTo %f, %f;\n"), i->x, i->y);
    }
    i = _pts.begin();
    trace(_t("@lineTo %f, %f;\n"), i->x, i->y);
    trace(_t("@@\n"));
#endif
}

void append_linestrips_rav(linestripvec& rav, linestrips& src)
{
    rav.reserve(rav.size() + src.size());
    for(painter_linestrip& ls : src)
        rav.push_back(&ls);
}

void create_linestrips_rav(linestripvec& rav, linestrips& src)
{
    rav.clear();
    append_linestrips_rav(rav, src);
}

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

void painter::set_font(const font& ft)
{
    scene* scn = scene::get_singleton_ptr();
    assert(scn);
    fontsys* fsys = scn->get_fontsys();
    assert(fsys);
    fsys->set_font(ft);
}

void painter::get_text_dimension(const gchar* str, int& w, int& h, int len)
{
    assert(str);
    scene* scn = scene::get_singleton_ptr();
    assert(scn);
    fontsys* fsys = scn->get_fontsys();
    assert(fsys);
    fsys->get_size(str, w, h, len);
}

void painter::draw_image(texture2d* img, float x, float y)
{
    assert(img);
    save();
    painter_brush brush;
    painter_extra_data ext;
    ext.reset(new painter_picture_data(img), [](painter_picture_data* p) { delete p; });
    brush.set_tag(painter_brush::picture);
    brush.set_extra(ext);
    set_brush(brush);
    int w, h;
    textureop::get_texture_dimension(img, w, h);
    draw_rect(rectf(x, y, (float)w, (float)h));
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

void painter::draw_text(const gchar* str, int x, int y, const color& cr, int length)
{
    if(!str || !length)
        return;
    scene* scn = scene::get_singleton_ptr();
    assert(scn);
    fontsys* fsys = scn->get_fontsys();
    assert(fsys);
    com_ptr<texture2d> tex;
    if(!fsys->create_text_texture(&tex, str, 0, 0, cr, length)) {
        assert(!"create text texture failed.");
        return;
    }
    assert(tex);
    _text_image_cache.push_back(tex.get());
    draw_image(tex.detach(), (float)x, (float)y);
}

void painter::destroy_text_image_cache()
{
    for(auto* img : _text_image_cache) {
        assert(img);
        release_texture2d(img);
    }
    _text_image_cache.clear();
}

painter_obj::painter_obj(const painter_context& ctx)
{
    set_context(ctx);
    _parent = 0;
}

painter_obj_iter painter_obj::add_child(painter_obj* p)
{
    assert(p);
    assert(!p->get_parent());
    p->set_parent(this);
    _children.push_back(p);
    return -- _children.end();
}

painter_obj_iter painter_obj::find_child_iterator(painter_obj* p)
{
    assert(p);
    assert(p->get_parent() == this);
    auto f = std::find(_children.begin(), _children.end(), p);
    return f;
}

painter_obj_iter painter_obj::add_child_before(painter_obj_iter pos, painter_obj* p)
{
    assert(p);
    assert(!p->get_parent());
    p->set_parent(this);
    return _children.insert(pos, p);
}

painter_obj_iter painter_obj::add_child_after(painter_obj_iter pos, painter_obj* p)
{
    assert(p);
    assert(pos != _children.end());
    assert(!p->get_parent());
    p->set_parent(this);
    return _children.insert(++ pos, p);
}

void painter_obj::detach_child(painter_obj_iter pos)
{
    assert(pos != _children.end());
    auto* p = *pos;
    assert(p);
    assert(p->get_parent() == this);
    p->set_parent(0);
    _children.erase(pos);
}

void painter_obj::detach_child(painter_obj* p)
{
    assert(p && (p->get_parent() == this));
    auto f = find_child_iterator(p);
    assert(f != _children.end());
    detach_child(f);
}

void painter_obj::remove_child(painter_obj_iter pos)
{
    assert(pos != _children.end());
    auto* p = *pos;
    assert(p && (p->get_parent() == this));
    detach_child(pos);
    delete p;
}

void painter_obj::remove_child(painter_obj* p)
{
    assert(p && (p->get_parent() == this));
    auto f = find_child_iterator(p);
    assert(f != _children.end());
    remove_child(f);
}

painter_obj_iter painter_obj::get_self_iterator()
{
    assert(_parent);
    return _parent->find_child_iterator(this);
}

void painter_obj::destroy_children()
{
    for(auto* p : _children)
        delete p;
    _children.clear();
}

painter_obj::~painter_obj()
{
    _parent = nullptr;
    destroy_children();
}

__ariel_end__
