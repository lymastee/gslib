/*
 * Copyright (c) 2016-2020 lymastee, All rights reserved.
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

#ifndef clip2_84d62df0_3a5c_4f7c_81dc_20b38c3b391c_h
#define clip2_84d62df0_3a5c_4f7c_81dc_20b38c3b391c_h

#include <gslib/std.h>
#include <gslib/tree.h>
#include <gslib/rtree.h>
#include <gslib/utility.h>
#include <ariel/raster.h>

__ariel_begin__

class clip_edge;
class clip_src_edge;
class clip_loop;
class clip_strip;
class clipper;

typedef unordered_set<clip_edge*> clip_edge_table;
typedef unordered_set<clip_strip*> clip_strip_table;
typedef list<clip_edge> clip_edges;
typedef vector<clip_edge*> clip_edge_ptrs;
typedef vector<clip_src_edge*> clip_src_edges;

typedef rtree_entity<clip_edge*> clip_rtree_entity;
typedef rtree_node<clip_rtree_entity> clip_rtree_node;
typedef _tree_allocator<clip_rtree_node> clip_rtree_alloc;
typedef tree<clip_rtree_entity, clip_rtree_node, clip_rtree_alloc> clip_tree;
typedef rtree<clip_rtree_entity, quadratic_split_alg<13, 5, clip_tree>, clip_rtree_node, clip_rtree_alloc> clip_rtree;

typedef _treenode_wrapper<clip_loop> clip_loop_tree_wrapper;
typedef tree<clip_loop, clip_loop_tree_wrapper> clip_loop_tree;
typedef clip_loop_tree::iterator clip_loop_tree_iter;

class __gs_novtable clip_src_edge abstract
{
public:
    enum type
    {
        st_line,
        st_quad,
        st_cubic,
    };

public:
    virtual ~clip_src_edge() {}
    virtual type get_type() const = 0;
    virtual const vec2& get_point(int i) const = 0;
    virtual const vec2& get_point_rev(int i) const = 0;
    virtual const vec2& get_begin_point() const = 0;
    virtual const vec2& get_end_point() const = 0;
    virtual void tracing() const = 0;
    const rectf& get_bound_rect() const { return _bound; }

protected:
    rectf               _bound;
};

class clip_src_edge_line:
    public clip_src_edge
{
public:
    clip_src_edge_line(const vec2& p0, const vec2& p1);
    virtual type get_type() const override { return st_line; }
    virtual const vec2& get_point(int i) const override;
    virtual const vec2& get_point_rev(int i) const override;
    virtual const vec2& get_begin_point() const override { return _pt[0]; }
    virtual const vec2& get_end_point() const override { return _pt[1]; }
    virtual void tracing() const override;

private:
    vec2                _pt[2];
};

class clip_src_edge_quad:
    public clip_src_edge
{
public:
    clip_src_edge_quad(const vec2& p0, const vec2& p1, const vec2& p2);
    virtual type get_type() const override { return st_quad; }
    virtual const vec2& get_point(int i) const override;
    virtual const vec2& get_point_rev(int i) const override;
    virtual const vec2& get_begin_point() const override { return _pt[0]; }
    virtual const vec2& get_end_point() const override { return _pt[2]; }
    virtual void tracing() const override;

private:
    vec2                _pt[3];
};

class clip_src_edge_cubic:
    public clip_src_edge
{
public:
    clip_src_edge_cubic(const vec2& p0, const vec2& p1, const vec2& p2, const vec2& p3);
    virtual type get_type() const override { return st_cubic; }
    virtual const vec2& get_point(int i) const override;
    virtual const vec2& get_point_rev(int i) const override;
    virtual const vec2& get_begin_point() const override { return _pt[0]; }
    virtual const vec2& get_end_point() const override { return _pt[3]; }
    virtual void tracing() const override;

private:
    vec2                _pt[4];
};

class clip_src_edge_fetcher
{
public:
    clip_src_edge_fetcher(clip_src_edge* e) { _src = e; }
    virtual ~clip_src_edge_fetcher() {}
    virtual const vec2& get_point(int i) const { return _src->get_point(i); }
    virtual const vec2& get_begin_point() const { return _src->get_begin_point(); }
    virtual const vec2& get_end_point() const { return _src->get_end_point(); }
    clip_src_edge::type get_type() const { return _src->get_type(); }
    const rectf& get_bound_rect() const { return _src->get_bound_rect(); }
    void tracing() const { _src->tracing(); }

protected:
    clip_src_edge*      _src = nullptr;
};

class clip_src_edge_fetcher_rev:
    public clip_src_edge_fetcher
{
public:
    clip_src_edge_fetcher_rev(clip_src_edge* e): clip_src_edge_fetcher(e) {}
    virtual const vec2& get_point(int i) const override { return _src->get_point_rev(i); }
    virtual const vec2& get_begin_point() const override { return _src->get_end_point(); }
    virtual const vec2& get_end_point() const override { return _src->get_begin_point(); }
};

class clip_edge
{
public:
    typedef clip_src_edge_fetcher src_edge_fetcher;

public:
    clip_edge() {}
    clip_edge(clip_loop* lp): _loop(lp) {}
    ~clip_edge();
    void set_loop(clip_loop* lp) { _loop = lp; }
    void set_org(const vec2& v) { _org = v; }
    void set_dest(const vec2& v) { _symmetric->set_org(v); }
    void set_src_edge(clip_src_edge* e, bool rev);
    void setup_src_edge(clip_src_edge* src);
    void set_prev(clip_edge* e) { _prev = e; }
    void set_next(clip_edge* e) { _next = e; }
    void set_symmetric(clip_edge* e) { _symmetric = e; }
    void set_id(int id);
    void set_id(const vector<int>& ids, int id);
    const vector<int>& get_ids() const { return _ids; }
    clip_loop* get_loop() const { return _loop; }
    clip_loop* find_host_loop() const;
    const vec2& get_org() const { return _org; }
    const vec2& get_dest() const { return _symmetric->get_org(); }
    src_edge_fetcher* get_sef() const { return _sef; }
    clip_edge* get_prev() const { return _prev; }
    clip_edge* get_next() const { return _next; }
    clip_edge* get_symmetric() const { return _symmetric; }
    void tracing() const { _sef->tracing(); }

protected:
    clip_loop*          _loop = nullptr;        /* only head may have this */
    src_edge_fetcher*   _sef = nullptr;
    clip_edge*          _prev = nullptr;
    clip_edge*          _next = nullptr;
    clip_edge*          _symmetric = nullptr;
    vec2                _org;
    vector<int>         _ids;
};

class clip_loop
{
public:
    clip_loop() {}
    clip_loop(clip_edge* e): _begin(e) {}
    virtual ~clip_loop() {}
    virtual bool is_strip() const { return false; }
    virtual void fix_children();    // todo: remove
    virtual void fix_edge_id();
    virtual void fix_scraps(clipper& c) {}  // todo
    virtual void tracing() const;

protected:
    clip_edge*          _begin = nullptr;

public:
    void set_begin(clip_edge* e) { _begin = e; }
    clip_edge* get_begin() const { return _begin; }

public:
    template<class _lamb>
    void for_each(_lamb lamb) const
    {
        if(!_begin)
            return;
        lamb(_begin);
        for(auto* e = _begin->get_next(); e && e != _begin; e = e->get_next())
            lamb(e);
    }
};

class clip_strip:
    public clip_loop
{
public:
    clip_strip() {}
    clip_strip(clip_edge* e): clip_loop(e) { set_end(e); }
    virtual bool is_strip() const override { return true; }
    virtual void fix_children() override;
    virtual void fix_edge_id() override;
    virtual void tracing() const override;

protected:
    clip_edge*          _end = nullptr;
    bool                _is_complete = false;

public:
    void set_end(clip_edge* e) { _end = e; }
    clip_edge* get_end() const { return _end; }
    const vec2& get_begin_point() const { return _begin->get_org(); }
    const vec2& get_end_point() const { return _end->get_dest(); }
    void fix_end(clip_edge* e);
    void set_complete(bool b) { _is_complete = b; }
    bool is_complete() const { return _is_complete; }
    void init(clip_edge* e);
    void setup(clip_edge* e);
    void push_front(clip_edge* e);
    void push_back(clip_edge* e);
    bool try_finish();
    void finish();

public:
    template<class _lamb>
    void for_each(_lamb lamb) const
    {
        if(!_begin)
            return;
        lamb(_begin);
        for(auto* e = _begin->get_next(); e && e != _end; e = e->get_next())
            lamb(e);
        if(_end && (_begin != _end))
            lamb(_end);
    }
};

class clipper
{
    friend class clip_proc_intersections;
    friend class clip_loop;
    friend class clip_strip;

public:
    clipper() {}
    ~clipper() { reset(); }
    void reset();
    void add_path(const painter_path& path);
    void tracing() const;

protected:
    clip_edges          _edge_holdings;
    clip_src_edges      _src_edge_holdings;
    clip_strip_table    _coarse_strips;
    clip_rtree          _rtree;

protected:
    int clip_coarse(const painter_path& path, int start);
    int proc_line_edge(clip_strip* strip, int eid, const vec2& last_pt, const painter_node* node);
    int proc_quad_edge(clip_strip* strip, int eid, const vec2& last_pt, const painter_path::quad_to_node* node);
    int proc_cubic_edge(clip_strip* strip, int eid, const vec2& last_pt, const painter_path::cubic_to_node* node);
    int proc_cubic_edge_nosi(clip_strip* strip, int eid, const vec2& p0, const vec2& p1, const vec2& p2, const vec2& p3);

protected:
    clip_edge* add_edge(const vec2& last_pt, const vec2& pt);
    clip_edge* add_line_edge(const vec2& p0, const vec2& p1);
    clip_edge* add_quad_edge(const vec2& p0, const vec2& p1, const vec2& p2);
    clip_edge* add_cubic_edge(const vec2& p0, const vec2& p1, const vec2& p2, const vec2& p3);
};

__ariel_end__

#endif
