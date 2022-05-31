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

#pragma once

#ifndef painterport_eca0917d_5109_4de2_ae6f_18d5c163d02c_h
#define painterport_eca0917d_5109_4de2_ae6f_18d5c163d02c_h

#include <gslib/rtree.h>
#include <ariel/painter.h>

__ariel_begin__

class painter_obj;

typedef rtree_entity<painter_obj*> painter_obj_entity;
typedef rtree_node<painter_obj_entity> painter_obj_node;
typedef _tree_allocator<painter_obj_node> painter_obj_alloc;
typedef tree<painter_obj_entity, painter_obj_node, painter_obj_alloc> painter_obj_tree;
typedef rtree<painter_obj_entity, quadratic_split_alg<25, 10, painter_obj_tree>, painter_obj_node, painter_obj_alloc> painter_obj_rtree;
typedef vector<painter_obj*> painter_objs;

class __gs_novtable painter_obj abstract
{
public:
    enum type
    {
        po_line,
        po_rect,
        po_path,
        po_text,
    };

public:
    painter_obj(const painter_context& ctx): _ctx(ctx) {}
    virtual ~painter_obj() {}
    virtual type get_type() const = 0;
    virtual rectf& get_rect(rectf& rc) const = 0;

protected:
    painter_context         _ctx;

public:
    const painter_context& get_context() const { return _ctx; }
};

class painter_line_obj:
    public painter_obj
{
public:
    painter_line_obj(const painter_context& ctx, const pointf& p1, const pointf& p2): painter_obj(ctx), _p1(p1), _p2(p2) {}
    virtual type get_type() const override { return po_line; }
    virtual rectf& get_rect(rectf& rc) const override;

private:
    pointf                  _p1;
    pointf                  _p2;
};

class painter_rect_obj:
    public painter_obj
{
public:
    painter_rect_obj(const painter_context& ctx,  const rectf& rc): painter_obj(ctx), _rc(rc) {}
    virtual type get_type() const override { return po_rect; }
    virtual rectf& get_rect(rectf& rc) const override { return rc = _rc; }

private:
    rectf                   _rc;
};

class painter_path_obj:
    public painter_obj
{
public:
    painter_path_obj(const painter_context& ctx, painter_path& take_me): painter_obj(ctx) { _path.swap(take_me); }
    painter_path_obj(const painter_context& ctx, const painter_path& path): painter_obj(ctx), _path(path) {}
    virtual type get_type() const override { return po_path; }
    virtual rectf& get_rect(rectf& rc) const override;
    rectf& rebuild_rect(rectf& rc) const;

private:
    painter_path            _path;
    mutable bool            _rc_valid = false;
    mutable rectf           _rc;
};

class painter_text_obj:
    public painter_obj
{
public:
    painter_text_obj(const painter_context& ctx, const string& txt, const pointf& p, const color& cr): painter_obj(ctx), _text(txt), _pos(p), _cr(cr) {}
    virtual type get_type() const override { return po_text; }
    virtual rectf& get_rect(rectf& rc) const override;

private:
    string                  _text;
    pointf                  _pos;
    color                   _cr;
};

class painterport:
    public painter
{
public:
    virtual ~painterport();
    virtual void resize(int w, int h) override {}
    virtual void draw_path(const painter_path& path) override;
    virtual void draw_line(const vec2& p1, const vec2& p2) override;
    virtual void draw_rect(const rectf& rc) override;
    virtual void draw_text(const gchar* str, float x, float y, const color& cr, int length) override;
    virtual void on_draw_begin() override;
    virtual void draw_viewport(painter* paint, const rectf& rc) {}

protected:
    painter_obj_rtree       _rtree;

public:
    rectf& get_area_rect(rectf& rc) const;
    void query_objs(painter_objs& objs, const rectf& rc);

protected:
    void add_obj(painter_obj* obj);
    void clear_objs();
};

__ariel_end__

#endif
