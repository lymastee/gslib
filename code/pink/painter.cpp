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

#include <assert.h>
#include <gslib/error.h>
#include <pink/painter.h>
#include <pink/utility.h>

__pink_begin__

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

void painter_linestrip::finish()
{
    if(_pts.front() == _pts.back()) {
        _pts.pop_back();
        set_closed(true);
    }
}

bool painter_linestrip::is_clock_wise() const
{
    if(!_closed || _pts.size() <= 2)
        return false;
    if(is_clockwise_init())
        return _is_clock_wise;
    set_clockwise_init();
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
    if(is_convex_init())
        return _is_convex;
    set_convex_init();
    bool cw = is_clock_wise();
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
    bool cw = is_clock_wise();
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
    _parent = 0;
    destroy_children();
}

software_painter::software_painter()
{
    _image = 0;
    _dirty = 0;
    _locked = false;
}

software_painter::~software_painter()
{
    /* do nothing */
}

image* software_painter::select(image* ptr)
{
    image* prev = _image;
    _image = ptr;
    return prev;
}

bool software_painter::lock(const rect& rc1)
{
    assert(_image && !_locked);
    int w = _image->get_width(), h = _image->get_height();
    rect rc = rc1;
    if(rc.right > w)
        rc.right = w;
    if(rc.bottom > h)
        rc.bottom = h;
    if(rc.right <= rc.left || rc.bottom <= rc.top)
        return false;
    _lock = rc;
    _locked = true;
    return true;
}

const rect& software_painter::unlock()
{
    _locked = false;
    return _lock;
}

void software_painter::draw(const image* img, int x, int y, int cx, int cy, int sx, int sy)
{
    assert(_image && _locked);
    x += _lock.left;
    y += _lock.top;
    if(x < 0) { sx -= x; cx += x; x = 0; }
    if(y < 0) { sy -= y; cy += y; y = 0; }
    if(x + cx > _lock.right)
        cx = _lock.right - x;
    if(y + cy > _lock.bottom)
        cy = _lock.bottom - y;
    if(!cx || !cy)
        return;
    //_image->draw(img, x, y, cx, cy, sx, sy);
    // todo:
}

void software_painter::draw_text(const gchar* str, int x, int y, const pixel& p)
{
    assert(str && _locked);
    x += _lock.left;
    y += _lock.top;
    assert(x >= 0 && y >= 0);
    //_image->draw_text(str, x, y, p);
    // todo:
}

void software_painter::draw_line(const point& start, const point& end, const pixel& p)
{
    assert(_locked);
    point ps = start, pe = end;
    ps.offset(_lock.left, _lock.top);
    pe.offset(_lock.left, _lock.top);
    //_image->draw_line(ps, pe, p);
    // todo:
}

void software_painter::draw_rect(const rect& rc, const pixel& p)
{
    assert(_locked);
    rect r = rc;
    r.offset(_lock.left, _lock.top);
    //_image->draw_rect(r, p);
    // todo:
}

__pink_end__
