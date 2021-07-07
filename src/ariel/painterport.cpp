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

#include <ariel/scene.h>
#include <ariel/painterport.h>

__ariel_begin__

rectf& painter_line_obj::get_rect(rectf& rc) const
{
    rc.set_by_pts(_p1, _p2);
    return rc;
}

rectf& painter_path_obj::get_rect(rectf& rc) const
{
    if(_rc_valid)
        return rc = _rc;
    _path.get_boundary_box(_rc);
    _rc_valid = true;
    return rc = _rc;
}

rectf& painter_path_obj::rebuild_rect(rectf& rc) const
{
    _rc_valid = false;
    return get_rect(rc);
}

rectf& painter_text_obj::get_rect(rectf& rc) const
{
    scene* scn = scene::get_singleton_ptr();
    assert(scn);
    fontsys* fsys = scn->get_fontsys();
    assert(fsys);
    int w, h;
    fsys->query_size(_text.c_str(), w, h);
    rc.set_rect(_pos.x, _pos.y, (float)w, (float)h);
    return rc;
}

painterport::~painterport()
{
    clear_objs();
}

void painterport::draw_path(const painter_path& path)
{
    add_obj(new painter_path_obj(get_context(), path));
}

void painterport::draw_line(const vec2& p1, const vec2& p2)
{
    auto* obj = new painter_line_obj(get_context(), p1, p2);
    assert(obj);
    _rtree.insert_line(obj, p1, p2);
}

void painterport::draw_rect(const rectf& rc)
{
    add_obj(new painter_rect_obj(get_context(), rc));
}

void painterport::draw_text(const gchar* str, float x, float y, const color& cr, int length)
{
    add_obj(new painter_text_obj(get_context(), string(str, length), pointf(x, y), cr));
}

void painterport::on_draw_begin()
{
    __super::on_draw_begin();
    clear_objs();
}

rectf& painterport::get_area_rect(rectf& rc) const
{
    if(!_rtree.is_valid()) {
        rc.set_rect(0.f, 0.f, 0.f, 0.f);
        return rc;
    }
    auto r = _rtree.get_root();
    rc = r->const_rect();
    return rc;
}

void painterport::query_objs(painter_objs& objs, const rectf& rc)
{
    _rtree.query(rc, objs);
}

void painterport::add_obj(painter_obj* obj)
{
    assert(obj);
    rectf rc;
    obj->get_rect(rc);
    _rtree.insert(obj, rc);
}

void painterport::clear_objs()
{
    _rtree.for_each([](painter_obj_entity* ent) { delete ent->get_bind_arg(); });
    _rtree.destroy();
}

__ariel_end__
