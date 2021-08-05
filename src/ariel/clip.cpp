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

#include <ariel/clip.h>
#include <gslib/utility.h>
#include <gslib/error.h>

#undef min
#undef max

#include <clipper/clipper.hpp>
#include <clipper/clipper.cpp>

__ariel_begin__

using namespace ClipperLib;

#define int_scale_ratio     1000.f

class clip_edge;
class clip_point;
class clipper;
struct clip_ep_map_key;
struct clip_ep_map_hash;
struct clip_ep_map_value;

typedef vector<clip_edge*> clip_edges;
typedef vector<clip_point*> clip_points;
typedef map<clip_edge*, float> clip_intersect_info_map;
typedef unordered_map<clip_ep_map_key, clip_ep_map_value, clip_ep_map_hash> clip_ep_map;
typedef vector<Paths*> PathHoldings;

static IntPoint cvt_AJ_point(const vec2& pt) { return IntPoint(round(pt.x * int_scale_ratio), round(pt.y * int_scale_ratio)); }
static IntPoint cvt_AJ_point(const vec2& pt, clip_point* p) { return IntPoint(round(pt.x * int_scale_ratio), round(pt.y * int_scale_ratio), (cInt)p); }
static vec2 cvt_point(const IntPoint& pt) { return vec2((float)pt.X / int_scale_ratio, (float)pt.Y / int_scale_ratio); }
static vec2 trunc_point(const vec2& pt) { return cvt_point(cvt_AJ_point(pt)); }
static clip_point* get_clip_point(const IntPoint& pt) { return reinterpret_cast<clip_point*>(pt.Z); }

static void trace_AJ_path(const Path& path)
{
    if(path.empty())
        return;
    trace(_t("@!\n"));
    vec2 p0 = cvt_point(path.at(0));
    trace(_t("@moveTo %f, %f;\n"), p0.x, p0.y);
    for(int i = 1; i < (int)path.size(); i ++) {
        vec2 p = cvt_point(path.at(i));
        trace(_t("@lineTo %f, %f;\n"), p.x, p.y);
    }
    trace(_t("@lineTo %f, %f;\n"), p0.x, p0.y);
    for(const IntPoint& pt : path) {
        vec2 p = cvt_point(pt);
        trace(_t("@dot %f, %f;\n"), p.x, p.y);
    }
    trace(_t("@@\n"));
}

static void convert_to_polygons(painter_linestrips& polygons, const Paths& out)
{
    for(auto i = out.begin(); i != out.end(); ++i) {
        if(!i->empty()) {
            polygons.push_back(painter_linestrip());
            painter_linestrip& ls = polygons.back();
            for(int j = (int)i->size() - 1; j >= 0; j--)
                ls.add_point(cvt_point(i->at(j)));
        }
    }
}

static void convert_to_paths(painter_paths& paths, const Paths& out)
{
    for(auto i = out.begin(); i != out.end(); ++i) {
        if(!i->empty()) {
            paths.push_back(painter_path());
            painter_path& path = paths.back();
            path.move_to(cvt_point(i->front()));
            for(int j = (int)i->size() - 1; j > 0; j--)
                path.line_to(cvt_point(i->at(j)));
            path.close_path();
        }
    }
}

static void convert_to_clipper_paths(Paths& paths, const painter_linestrip& ls)
{
    int s = ls.get_size();
    if(s > 0) {
        paths.push_back(Path());
        Path& cp = paths.back();
        for(auto i = 0; i < s; i++)
            cp.push_back(cvt_AJ_point(ls.get_point(i)));
    }
}

static void convert_to_clipper_paths(Paths& paths, const painter_linestrips& lss)
{
    for(auto i = lss.begin(); i != lss.end(); ++i)
        convert_to_clipper_paths(paths, *i);
}

enum clip_edge_type
{
    et_linear,
    et_quad,
    et_cubic,
};

class __gs_novtable clip_edge abstract
{
public:
    typedef clip_edge_type type;

public:
    virtual ~clip_edge() {}
    virtual type get_type() const = 0;
    virtual int get_point_count() const = 0;
    virtual const vec2& get_point(int i) const = 0;
    const vec2& get_last_point() const { return get_point(get_point_count() - 1); }
};

template<clip_edge_type _type, int _count>
class clip_edge_tpl :
    public clip_edge
{
public:
    virtual type get_type() const override { return _type; }
    virtual int get_point_count() const override { return _count; }
    virtual const vec2& get_point(int i) const override
    {
        assert(i >= 0 && i < _count);
        return _pt[i];
    }
    void set_point(int i, const vec2& p)
    {
        assert(i >= 0 && i < _count);
        _pt[i] = p;
    }

private:
    vec2                _pt[_count];
};

struct clip_ep_map_key
{
    clip_point*         p1 = nullptr;
    clip_point*         p2 = nullptr;

    clip_ep_map_key(clip_point* cp1, clip_point* cp2)
    {
        assert(cp1 != cp2);
        p1 = cp1, p2 = cp2;
    }
    bool operator==(const clip_ep_map_key& k) const
    {
        return p1 == k.p1 && p2 == k.p2;
    }
};

struct clip_ep_map_hash
{
    size_t operator()(const clip_ep_map_key& k) const
    {
        assert(sizeof(clip_ep_map_key) == 8);
        return hash_bytes((const byte*)&k, sizeof(clip_ep_map_key));
    }
};

struct clip_ep_map_value
{
    clip_edge*          edge = nullptr;
    bool                reversed = false;

    clip_ep_map_value() { assert(!"error."); }
    clip_ep_map_value(clip_edge* e, bool r)
    {
        edge = e;
        reversed = r;
    }
};

typedef clip_edge_tpl<et_linear, 2> clip_linear_edge;
typedef clip_edge_tpl<et_quad, 3> clip_quad_edge;
typedef clip_edge_tpl<et_cubic, 4> clip_cubic_edge;

enum clip_point_type
{
    pt_end_point,
    pt_interpolate_point,
    pt_intersect_point,
};

class __gs_novtable clip_point abstract
{
public:
    typedef clip_point_type type;

public:
    virtual ~clip_point() {}
    virtual type get_type() const = 0;

protected:
    vec2                _pt;

public:
    void set_point(const vec2& pt) { _pt = pt; }
    const vec2& get_point() const { return _pt; }
};

class clip_end_point:
    public clip_point
{
public:
    virtual type get_type() const override { return pt_end_point; }

private:
    clip_edge*          _prev_edge = nullptr;
    clip_edge*          _next_edge = nullptr;

public:
    void set_prev_edge(clip_edge* e) { _prev_edge = e; }
    void set_next_edge(clip_edge* e) { _next_edge = e; }
    clip_edge* get_prev_edge() const { return _prev_edge; }
    clip_edge* get_next_edge() const { return _next_edge; }
};

class clip_interpolate_point:
    public clip_point
{
public:
    virtual type get_type() const override { return pt_interpolate_point; }

private:
    clip_edge*          _edge = nullptr;
    float               _ratio = 0.f;

public:
    void set_edge(clip_edge* e) { _edge = e; }
    clip_edge* get_edge() const { return _edge; }
    void set_ratio(float r) { _ratio = r; }
    float get_ratio() const { return _ratio; }
};

class clip_intersect_point:
    public clip_point
{
public:
    typedef clip_intersect_info_map intersect_info_map;
    typedef typename intersect_info_map::iterator intersect_iter;

public:
    virtual type get_type() const override { return pt_intersect_point; }

private:
    intersect_info_map  _itrs_map;

public:
    void add_intersect_info(clip_edge* e, float r) { _itrs_map.emplace(e, r); }
    const intersect_info_map& get_intersect_info_map() const { return _itrs_map; }
};

static clipper*  __current_clipper = nullptr;

static clip_edge* determine_edge(const clip_point* p1, const clip_point* p2)
{
    assert(p1 && p2);
    auto t1 = p1->get_type();
    auto t2 = p2->get_type();
    if(t1 == pt_interpolate_point)
        return static_cast<const clip_interpolate_point*>(p1)->get_edge();
    else if(t2 == pt_interpolate_point)
        return static_cast<const clip_interpolate_point*>(p2)->get_edge();
    else if(t1 == pt_end_point && t2 == pt_end_point) {
        auto ep1 = static_cast<const clip_end_point*>(p1);
        auto ep2 = static_cast<const clip_end_point*>(p2);
        if(ep1->get_next_edge() == ep2->get_prev_edge())
            return ep1->get_next_edge();
        else {
            assert(ep1->get_prev_edge() == ep2->get_next_edge());
            return ep1->get_prev_edge();
        }
    }
    else if(t1 == pt_end_point && t2 == pt_intersect_point) {
        auto ep1 = static_cast<const clip_end_point*>(p1);
        auto ip2 = static_cast<const clip_intersect_point*>(p2);
        const clip_intersect_info_map& iim = ip2->get_intersect_info_map();
        auto f1 = iim.find(ep1->get_next_edge());
        if(f1 != iim.end())
            return ep1->get_next_edge();
        assert(iim.find(ep1->get_prev_edge()) != iim.end());
        return ep1->get_prev_edge();
    }
    else if(t1 == pt_intersect_point && t2 == pt_end_point) {
        auto ip1 = static_cast<const clip_intersect_point*>(p1);
        auto ep2 = static_cast<const clip_end_point*>(p2);
        const clip_intersect_info_map& iim = ip1->get_intersect_info_map();
        auto f1 = iim.find(ep2->get_next_edge());
        if(f1 != iim.end())
            return ep2->get_next_edge();
        assert(iim.find(ep2->get_prev_edge()) != iim.end());
        return ep2->get_prev_edge();
    }
    else if(t1 == pt_intersect_point && t2 == pt_intersect_point) {
        auto ip1 = static_cast<const clip_intersect_point*>(p1);
        auto ip2 = static_cast<const clip_intersect_point*>(p2);
        const clip_intersect_info_map& iim1 = ip1->get_intersect_info_map();
        const clip_intersect_info_map& iim2 = ip2->get_intersect_info_map();
        for(const auto& p : iim1) {
            if(p.first->get_type() == et_linear) {  /* no footprint, means linear first. */
                auto f = iim2.find(p.first);
                if(f != iim2.end())
                    return p.first;
            }
        }
        for(const auto& p : iim1) {
            auto f = iim2.find(p.first);
            if(f != iim2.end())
                return p.first;
        }
    }
    assert(!"unexpected situation.");
    return nullptr;
}

class clipper
{
    friend void clip_zfill(IntPoint&, IntPoint&, IntPoint&, IntPoint&, IntPoint&);

public:
    clipper()
    {
        _AJ_clipper.StrictlySimple(true);
        _AJ_clipper.ZFillFunction(&clip_zfill);
        __current_clipper = this;
    }
    ~clipper()
    {
        for(auto* p : _edges)   delete p;
        for(auto* p : _points)  delete p;
        for(auto* p : _AJ_holdings) delete p;
        _edges.clear();
        _points.clear();
        _AJ_holdings.clear();
        __current_clipper = nullptr;
    }
    void add_path(const painter_path& path, PolyType poly_type)
    {
        _AJ_paths = new Paths;
        _AJ_holdings.push_back(_AJ_paths);
        painter_paths paths;
        path.to_sub_paths(paths);
        for(painter_path& sp : paths)
            add_sub_path(sp);
        _AJ_clipper.AddPaths(*_AJ_paths, poly_type, true);
        _AJ_paths = nullptr;
    }
    void add_sub_path(const painter_path& path)
    {
        if(path.size() < 2)
            return;
        auto* first = path.get_node(0);
        assert(first && first->get_tag() == painter_path::pt_moveto);
        auto* last = first;
        if(path.size() == 2) {
            auto* node = path.get_node(1);
            assert(node);
            if(node->get_tag() == painter_path::pt_moveto
                || node->get_tag() == painter_path::pt_lineto
                )
                return;
            switch(node->get_tag())
            {
            case painter_path::pt_moveto:
            case painter_path::pt_lineto:
                return;
            case painter_path::pt_quadto:
                add_single_quad_segment(last, static_cast<const painter_path::quad_to_node*>(node));
                return;
            case painter_path::pt_cubicto:
                add_single_cubic_segment(last, static_cast<const painter_path::cubic_to_node*>(node));
                return;
            }
        }
        else if(path.size() == 3) {
            auto* node2 = path.get_node(2);
            assert(node2);
            if(fuzz_cmp(last->get_point(), node2->get_point()) < 0.1f) {    /* closed situation */
                auto* node1 = path.get_node(1);
                assert(node1);
                if(node1->get_tag() == painter_path::pt_lineto) {
                    if(node2->get_tag() == painter_path::pt_lineto)
                        return; /* area 0 */
                    Path& ajp = begin_create_AJ_path();
                    auto* p1 = create_end_point(ajp, last->get_point());
                    auto* p2 = create_end_point(ajp, node1->get_point());
                    assert(p1 && p2);
                    add_linear_edge(p1, p2);
                    auto* p3 = split_curve_segment(ajp, p2, node1, node2);
                    assert(p3);
                    add_curve_edge(p3, p3->get_next_edge(), p1);
                    end_create_AJ_path(ajp);
                }
                else if(node2->get_tag() == painter_path::pt_lineto) {
                    Path& ajp = begin_create_AJ_path();
                    auto* p1 = create_end_point(ajp, last->get_point());
                    assert(p1);
                    auto* p2 = split_curve_segment(ajp, p1, last, node1);
                    auto* p3 = create_end_point(ajp, node1->get_point());
                    assert(p2 && p3);
                    add_curve_edge(p2, p2->get_next_edge(), p3);
                    add_linear_edge(p3, p1);
                    end_create_AJ_path(ajp);
                }
                else {  /* they are all curves, split the longer one. */
                    auto calc_control_length = [](const painter_node* last, const painter_node* node)-> float {
                        assert(last && node);
                        if(node->get_tag() == painter_path::pt_quadto) {
                            auto qnode = static_cast<const painter_path::quad_to_node*>(node);
                            return quad_control_length(last->get_point(), qnode->get_control(), qnode->get_point());
                        }
                        else if(node->get_tag() == painter_path::pt_cubicto) {
                            auto qnode = static_cast<const painter_path::cubic_to_node*>(node);
                            return cubic_control_length(last->get_point(), qnode->get_control1(), qnode->get_control2(), qnode->get_point());
                        }
                        return 0.f;
                    };
                    float len1 = calc_control_length(last, node1);
                    float len2 = calc_control_length(node1, node2);
                    Path& ajp = begin_create_AJ_path();
                    if(len1 >= len2) {
                        auto* p1 = create_end_point(ajp, last->get_point());
                        assert(p1);
                        auto* p2 = split_curve_segment(ajp, p1, last, node1);
                        auto* p3 = create_end_point(ajp, node1->get_point());
                        assert(p2 && p3);
                        add_curve_edge(p2, p2->get_next_edge(), p3);
                        auto* e2 = create_curve_edge(node1, node2);
                        assert(e2);
                        create_curve_points(ajp, e2);
                        add_curve_edge(p3, e2, p1);
                    }
                    else {
                        auto* p1 = create_end_point(ajp, last->get_point());
                        auto* e1 = create_curve_edge(last, node1);
                        assert(p1 && e1);
                        create_curve_points(ajp, e1);
                        auto* p2 = create_end_point(ajp, node1->get_point());
                        assert(p2);
                        add_curve_edge(p1, e1, p2);
                        auto* p3 = split_curve_segment(ajp, p2, node1, node2);
                        assert(p3);
                        add_curve_edge(p3, p3->get_next_edge(), p1);
                    }
                    end_create_AJ_path(ajp);
                }
                return;
            }
        }
        auto& ajp = begin_create_AJ_path();
        auto firstep = create_end_point(ajp, last->get_point());
        assert(firstep);
        auto lastep = firstep;
        for(int i = 1; i < path.size(); i ++) {
            auto* node = path.get_node(i);
            assert(node);
            switch(node->get_tag())
            {
            case painter_path::pt_lineto:
                {
                    auto ep = create_end_point(ajp, node->get_point());
                    assert(ep);
                    add_linear_edge(lastep, ep);
                    lastep = ep;
                    break;
                }
            case painter_path::pt_quadto:
                {
                    auto e = create_quad_edge(last, static_cast<const painter_path::quad_to_node*>(node));
                    assert(e);
                    create_quad_points(ajp, e);
                    auto ep = create_end_point(ajp, node->get_point());
                    assert(ep);
                    add_curve_edge(lastep, e, ep);
                    lastep = ep;
                    break;
                }
            case painter_path::pt_cubicto:
                {
                    auto e = create_cubic_edge(last, static_cast<const painter_path::cubic_to_node*>(node));
                    assert(e);
                    create_cubic_points(ajp, e);
                    auto ep = create_end_point(ajp, node->get_point());
                    assert(ep);
                    add_curve_edge(lastep, e, ep);
                    lastep = ep;
                    break;
                }
            }
            last = node;
        }
        assert(last);
        if(fuzz_cmp(last->get_point(), first->get_point()) >= 0.1f)
            add_linear_edge(lastep, firstep);
        end_create_AJ_path(ajp);
    }
    void simplify(clip_result& result)
    {
        PolyTree polys;
        _AJ_clipper.Execute(ctUnion, polys);
        convert_output(result, polys);
    }
    void do_union(clip_result& result)
    {
        PolyTree polys;
        _AJ_clipper.Execute(ctUnion, polys);
        convert_output(result, polys);
    }
    void do_intersect(clip_result& result)
    {
        PolyTree polys;
        _AJ_clipper.Execute(ctIntersection, polys);
        convert_output(result, polys);
    }
    void do_substract(clip_result& result)
    {
        PolyTree polys;
        _AJ_clipper.Execute(ctDifference, polys);
        convert_output(result, polys);
    }
    void do_exclude(clip_result& result)
    {
        PolyTree polys;
        _AJ_clipper.Execute(ctXor, polys);
        convert_output(result, polys);
    }
    void trace_AJ_paths() const
    {
        for(const Paths* paths : _AJ_holdings) {
            assert(paths);
            for(const Path& path : *paths)
                trace_AJ_path(path);
        }
    }

private:
    Clipper         _AJ_clipper;
    PathHoldings    _AJ_holdings;
    Paths*          _AJ_paths = nullptr;            /* it's damn stupid vector<vector<ctor>>! */
    Path            _path_to_add;
    bool            _path_taken = false;
    clip_edges      _edges;
    clip_points     _points;
    clip_ep_map     _ep_map;

private:
    Path& begin_create_AJ_path()
    {
        if(_path_taken) {
            assert(!"error.");
            static Path bad_path;
            return bad_path;
        }
        _path_taken = true;
        assert(_path_to_add.empty());
        return _path_to_add;
    }
    void end_create_AJ_path(Path& path)
    {
        if(!_path_taken) {
            assert(!"no path has been taken.");
            return;
        }
        if(&_path_to_add != &path) {
            assert(!"path mismatch.");
            return;
        }
        if(!_path_to_add.empty()) {
            assert(_AJ_paths);
            _AJ_paths->push_back(_path_to_add);
            _path_to_add.swap(Path());
        }
        _path_taken = false;
    }
    void add_single_quad_segment(const painter_node* last, const painter_path::quad_to_node* node)
    {
        assert(last && node);
        bool is_closed = fuzz_cmp(last->get_point(), node->get_point()) < 0.1f;
        is_closed ? add_close_single_quad_segment(node) :
            add_open_single_quad_segment(last, node);
    }
    void add_single_cubic_segment(const painter_node* last, const painter_path::cubic_to_node* node)
    {
        assert(last && node);
        bool is_closed = fuzz_cmp(last->get_point(), node->get_point()) < 0.1f;
        is_closed ? add_close_single_cubic_segment(node) :
            add_open_single_cubic_segment(last, node);
    }
    void add_open_single_quad_segment(const painter_node* last, const painter_path::quad_to_node* node)
    {
        assert(last && node);
        auto& ajp = begin_create_AJ_path();
        auto p1 = create_end_point(ajp, last->get_point());
        assert(p1);
        auto p2 = split_quad_segment(ajp, p1, last, node);
        auto p3 = create_end_point(ajp, node->get_point());
        assert(p2 && p3);
        add_curve_edge(p2, p2->get_next_edge(), p3);
        add_linear_edge(p3, p1);
        end_create_AJ_path(ajp);
    }
    void add_close_single_quad_segment(const painter_path::quad_to_node* node)
    {
        /* nothing to add. */
    }
    void add_open_single_cubic_segment(const painter_node* last, const painter_path::cubic_to_node* node)
    {
        assert(last && node);
        auto& ajp = begin_create_AJ_path();
        auto p1 = create_end_point(ajp, last->get_point());
        assert(p1);
        auto p2 = split_cubic_segment(ajp, p1, last, node);
        auto p3 = create_end_point(ajp, node->get_point());
        assert(p2 && p3);
        add_curve_edge(p2, p2->get_next_edge(), p3);
        add_linear_edge(p3, p1);
        end_create_AJ_path(ajp);
    }
    void add_close_single_cubic_segment(const painter_path::cubic_to_node* node)
    {
        assert(node);
        vec2 cpts[4] = { node->get_point(), node->get_control1(), node->get_control2(), node->get_point() }, spts[7];
        split_cubic_bezier(spts, cpts, 1.f / 3.f);
        auto e1 = create_cubic_edge(spts[0], spts[1], spts[2], spts[3]);
        assert(e1);
        memcpy_s(cpts, sizeof(cpts), &spts[3], sizeof(cpts));
        split_cubic_bezier(spts, cpts, 0.5f);
        auto e2 = create_cubic_edge(spts[0], spts[1], spts[2], spts[3]);
        auto e3 = create_cubic_edge(spts[3], spts[4], spts[5], spts[6]);
        assert(e2 && e3);
        auto& ajp = begin_create_AJ_path();
        auto p1 = create_end_point(ajp, e1->get_point(0));
        assert(p1);
        create_cubic_points(ajp, e1);
        auto p2 = create_end_point(ajp, e2->get_point(0));
        assert(p2);
        create_cubic_points(ajp, e2);
        auto p3 = create_end_point(ajp, e3->get_point(0));
        assert(p3);
        create_cubic_points(ajp, e3);
        add_curve_edge(p1, e1, p2);
        add_curve_edge(p2, e2, p3);
        add_curve_edge(p3, e3, p1);
        end_create_AJ_path(ajp);
    }
    void split_quad_segment(clip_quad_edge* spe[2], const painter_node* last, const painter_path::quad_to_node* node)
    {
        assert(spe && last && node);
        vec2 qpts[3] = { last->get_point(), node->get_control(), node->get_point() }, spts[5];
        split_quad_bezier(spts, qpts, 0.5f);
        spe[0] = create_quad_edge(spts[0], spts[1], spts[2]);
        spe[1] = create_quad_edge(spts[2], spts[3], spts[4]);
    }
    void split_cubic_segment(clip_cubic_edge* spe[2], const painter_node* last, const painter_path::cubic_to_node* node)
    {
        assert(spe && last && node);
        vec2 cpts[4] = { last->get_point(), node->get_control1(), node->get_control2(), node->get_point() }, spts[7];
        split_cubic_bezier(spts, cpts, 0.5f);
        spe[0] = create_cubic_edge(spts[0], spts[1], spts[2], spts[3]);
        spe[1] = create_cubic_edge(spts[3], spts[4], spts[5], spts[6]);
    }
    clip_end_point* split_quad_segment(Path& path, clip_end_point* p0, const painter_node* last, const painter_path::quad_to_node* node)
    {
        assert(p0 && last && node);
        clip_quad_edge* spe[2];
        split_quad_segment(spe, last, node);
        create_quad_points(path, spe[0]);
        auto* p1 = create_end_point(path, spe[1]->get_point(0));
        assert(p1);
        add_curve_edge(p0, spe[0], p1);
        create_quad_points(path, spe[1]);
        p1->set_next_edge(spe[1]);
        return p1;
    }
    clip_end_point* split_cubic_segment(Path& path, clip_end_point* p0, const painter_node* last, const painter_path::cubic_to_node* node)
    {
        assert(p0 && last && node);
        clip_cubic_edge* spe[2];
        split_cubic_segment(spe, last, node);
        create_cubic_points(path, spe[0]);
        auto* p1 = create_end_point(path, spe[1]->get_point(0));
        assert(p1);
        add_curve_edge(p0, spe[0], p1);
        create_cubic_points(path, spe[1]);
        p1->set_next_edge(spe[1]);
        return p1;
    }
    clip_end_point* split_curve_segment(Path& path, clip_end_point* p0, const painter_node* last, const painter_node* node)
    {
        assert(p0 && last && node);
        switch(node->get_tag())
        {
        case painter_path::pt_quadto:
            return split_quad_segment(path, p0, last, static_cast<const painter_path::quad_to_node*>(node));
        case painter_path::pt_cubicto:
            return split_cubic_segment(path, p0, last, static_cast<const painter_path::cubic_to_node*>(node));
        default:
            assert(!"must be curve.");
            return nullptr;
        }
    }
    clip_linear_edge* create_linear_edge(clip_end_point* p1, clip_end_point* p2)
    {
        assert(p1 && p2);
        return create_linear_edge(p1->get_point(), p2->get_point());
    }
    clip_linear_edge* create_linear_edge(const painter_node* last, const painter_path::line_to_node* node)
    {
        assert(last && node);
        return create_linear_edge(last->get_point(), node->get_point());
    }
    clip_quad_edge* create_quad_edge(const painter_node* last, const painter_path::quad_to_node* node)
    {
        assert(last && node);
        return create_quad_edge(last->get_point(), node->get_control(), node->get_point());
    }
    clip_cubic_edge* create_cubic_edge(const painter_node* last, const painter_path::cubic_to_node* node)
    {
        assert(last && node);
        return create_cubic_edge(last->get_point(), node->get_control1(), node->get_control2(), node->get_point());
    }
    clip_edge* create_curve_edge(const painter_node* last, const painter_node* node)
    {
        assert(last && node);
        switch(node->get_tag())
        {
        case painter_path::pt_quadto:
            return create_quad_edge(last, static_cast<const painter_path::quad_to_node*>(node));
        case painter_path::pt_cubicto:
            return create_cubic_edge(last, static_cast<const painter_path::cubic_to_node*>(node));
        default:
            assert(!"must be curve.");
            return nullptr;
        }
    }
    clip_linear_edge* create_linear_edge(const vec2& p1, const vec2& p2)
    {
        auto* edge = new clip_linear_edge;
        assert(edge);
        _edges.push_back(edge);
        edge->set_point(0, p1);
        edge->set_point(1, p2);
        return edge;
    }
    clip_quad_edge* create_quad_edge(const vec2& p1, const vec2& p2, const vec2& p3)
    {
        auto* edge = new clip_quad_edge;
        assert(edge);
        _edges.push_back(edge);
        edge->set_point(0, p1);
        edge->set_point(1, p2);
        edge->set_point(2, p3);
        return edge;
    }
    clip_cubic_edge* create_cubic_edge(const vec2& p1, const vec2& p2, const vec2& p3, const vec2& p4)
    {
        auto* edge = new clip_cubic_edge;
        assert(edge);
        _edges.push_back(edge);
        edge->set_point(0, p1);
        edge->set_point(1, p2);
        edge->set_point(2, p3);
        edge->set_point(3, p4);
        return edge;
    }
    clip_end_point* create_end_point(Path& path, const vec2& p)
    {
        auto ep = new clip_end_point;
        assert(ep);
        _points.push_back(ep);
        path.push_back(cvt_AJ_point(p, ep));
        ep->set_point(p);
        return ep;
    }
    clip_intersect_point* create_intersect_point(const vec2& p)
    {
        auto ip = new clip_intersect_point;
        assert(ip);
        _points.push_back(ip);
        ip->set_point(p);
        return ip;
    }
    int create_quad_points(Path& path, clip_quad_edge* edge)
    {
        assert(edge);
        vec3 para[2];
        get_quad_parameter_equation(para, edge->get_point(0), edge->get_point(1), edge->get_point(2));
        int step = get_rough_interpolate_step(edge->get_point(0), edge->get_point(1), edge->get_point(2));
        float t, chord;
        t = chord = 1.f / (step - 1);
        for(int i = 1; i < step - 1; i ++, t += chord) {
            vec2 p;
            eval_quad(p, para, t);
            auto* ip = new clip_interpolate_point;
            assert(ip);
            _points.push_back(ip);
            path.push_back(cvt_AJ_point(p, ip));
            ip->set_point(p);
            ip->set_edge(edge);
            ip->set_ratio(t);
        }
        return _points.size();
    }
    int create_cubic_points(Path& path, clip_cubic_edge* edge)
    {
        assert(edge);
        vec4 para[2];
        get_cubic_parameter_equation(para, edge->get_point(0), edge->get_point(1), edge->get_point(2), edge->get_point(3));
        int step = get_rough_interpolate_step(edge->get_point(0), edge->get_point(1), edge->get_point(2), edge->get_point(3));
        float t, chord;
        t = chord = 1.f / (step - 1);
        for(int i = 1; i < step - 1; i ++, t += chord) {
            vec2 p;
            eval_cubic(p, para, t);
            auto* ip = new clip_interpolate_point;
            assert(ip);
            _points.push_back(ip);
            path.push_back(cvt_AJ_point(p, ip));
            ip->set_point(p);
            ip->set_edge(edge);
            ip->set_ratio(t);
        }
        return _points.size();
    }
    int create_curve_points(Path& path, clip_edge* edge)
    {
        assert(edge);
        switch(edge->get_type())
        {
        case et_quad:
            return create_quad_points(path, static_cast<clip_quad_edge*>(edge));
        case et_cubic:
            return create_cubic_points(path, static_cast<clip_cubic_edge*>(edge));
        }
        return 0;
    }
    void add_ep_map(clip_point* p1, clip_point* p2, clip_edge* edge)
    {
        assert(p1 && p2 && edge);
        _ep_map.emplace(clip_ep_map_key(p1, p2), clip_ep_map_value(edge, false));
        _ep_map.emplace(clip_ep_map_key(p2, p1), clip_ep_map_value(edge, true));
    }
    void add_linear_edge(clip_end_point* p1, clip_end_point* p2)
    {
        assert(p1 && p2);
        auto* edge = create_linear_edge(p1, p2);
        assert(edge);
        add_curve_edge(p1, edge, p2);
    }
    void add_curve_edge(clip_end_point* p1, clip_edge* edge, clip_end_point* p2)
    {
        assert(p1 && edge && p2);
        p1->set_next_edge(edge);
        p2->set_prev_edge(edge);
        add_ep_map(p1, p2, edge);
    }
    void convert_output(clip_result& result, PolyTree& poly_tree)
    {
        convert_output(result, result.birth(nullptr), &poly_tree);
    }
    void convert_output(painter_paths& output, Paths& paths)
    {
        for(Path& path : paths) {
            if(path.empty())
                continue;
            output.push_back(painter_path());
            painter_path& out_path = output.back();
            convert_output(out_path, path);
        }
    }
    void convert_output(clip_result& result, clip_result_iter iter, PolyNode* poly)
    {
        assert(poly);
        Path& path = poly->Contour;
        if(!path.empty())
            convert_output(*iter, path);
        if(poly->ChildCount() <= 0)
            return;
        auto r = result.birth_tail(iter);
        assert(r);
        auto node = poly->Childs.front();
        assert(node);
        convert_output(result, r, node);
        for(node = node->GetNext(); node; node = node->GetNext()) {
            r = result.insert_after(r);
            convert_output(result, r, node);
        }
    }
    void convert_output(painter_path& output, Path& path)
    {
        assert(!path.empty());
        ReversePath(path);
        auto skip_interpolate_points = [&path](int s)-> int {
            int len = (int)path.size();
            assert(s < len);
            for(int i = 0; i < len; ++ i, ++ s) {
                auto cp = get_clip_point(path.at(s));
                assert(cp);
                if(cp->get_type() != pt_interpolate_point)
                    return s;
            }
            assert(!"error.");
            return -1;
        };
        clip_edge* footprint = nullptr;
        auto skip_interpolate_points_till = [&path, &footprint](int s, int& c)-> int {
            int len = (int)path.size();
            assert(s < len);
            for(; c < len; ++ c, ++ s %= len) {
                auto cp = get_clip_point(path.at(s));
                assert(cp);
                if(cp->get_type() != pt_interpolate_point)
                    return s;
                footprint = static_cast<const clip_interpolate_point*>(cp)->get_edge();
            }
            return s;
        };
        int s = skip_interpolate_points(0);
        if(s < 0) {
            assert(!"unexpected.");
            return;
        }
        auto first = get_clip_point(path.at(s));
        assert(first);
        int start = output.size();
        output.move_to(first->get_point());
        int c = 0, last = s;
        for(;;) {
            footprint = nullptr;
            int n = skip_interpolate_points_till((last + 1) % path.size(), c);
            auto p1 = get_clip_point(path.at(last));
            auto p2 = get_clip_point(path.at(n));
            assert(p1 && p2);
            auto r = _ep_map.find(clip_ep_map_key(p1, p2));
            if(r != _ep_map.end())
                append_output(output, r->second.edge, r->second.reversed);
            else {
                if(!footprint)
                    footprint = determine_edge(p1, p2);
                assert(footprint);
                cut_edge(output, footprint, p1, p2);
            }
            if(n == s)
                break;
            last = n;
        }
        fix_ending(output, output.get_node(start));
    }
    void fix_ending(painter_path& path, const painter_node* start)
    {
        assert(start);
        painter_node* last = path.get_node(path.size() - 1);
        assert(last);
        switch(last->get_tag())
        {
        case painter_path::pt_lineto:
        case painter_path::pt_quadto:
        case painter_path::pt_cubicto:
            last->set_point(start->get_point());
            break;
        }
    }
    void append_output(painter_path& output, const clip_edge* edge, bool reversed)
    {
        assert(edge);
        switch(edge->get_type())
        {
        case et_linear:
            reversed ? output.line_to(edge->get_point(0)) : output.line_to(edge->get_point(1));
            break;
        case et_quad:
            reversed ? output.quad_to(edge->get_point(1), edge->get_point(0)) : output.quad_to(edge->get_point(1), edge->get_point(2));
            break;
        case et_cubic:
            reversed ? output.cubic_to(edge->get_point(2), edge->get_point(1), edge->get_point(0)) : output.cubic_to(edge->get_point(1), edge->get_point(2), edge->get_point(3));
            break;
        }
    }
    void cut_edge(painter_path& output, clip_edge* edge, clip_point* p1, clip_point* p2)
    {
        assert(edge && p1 && p2);
        if(edge->get_type() == et_linear) {
            output.line_to(p2->get_point());
            return;
        }
        float t1 = 0.f, t2 = 0.f;
        bool is_end1 = false, is_end2 = false;
        if(p1->get_type() == pt_end_point) {
            is_end1 = true;
            t1 = fuzz_cmp(edge->get_point(0), p1->get_point()) < 0.1f ? 0.f : 1.f;
        }
        else {
            assert(p1->get_type() == pt_intersect_point);
            auto p = static_cast<const clip_intersect_point*>(p1);
            t1 = p->get_intersect_info_map().find(edge)->second;
        }
        if(p2->get_type() == pt_end_point) {
            is_end2 = true;
            t2 = fuzz_cmp(edge->get_last_point(), p2->get_point()) < 0.1f ? 1.f : 0.f;
        }
        else {
            assert(p2->get_type() == pt_intersect_point);
            auto p = static_cast<const clip_intersect_point*>(p2);
            t2 = p->get_intersect_info_map().find(edge)->second;
        }
        bool reversed = false;
        if(t1 > t2) {
            gs_swap(p1, p2);
            gs_swap(t1, t2);
            gs_swap(is_end1, is_end2);
            reversed = true;
        }
        assert(!(is_end1 && is_end2));
        if(is_end1) {
            auto e = split_front_part(edge, t2);
            assert(e);
            append_output(output, e, reversed);
        }
        else if(is_end2) {
            auto e = split_back_part(edge, t1);
            assert(e);
            append_output(output, e, reversed);
        }
        else {
            auto e = split_middle_part(edge, t1, t2);
            assert(e);
            append_output(output, e, reversed);
        }
    }
    clip_edge* split_front_part(clip_edge* edge, float t)
    {
        assert(edge);
        switch(edge->get_type())
        {
        case et_linear:
            {
                auto e = create_linear_edge(edge->get_point(0), vec2().lerp(edge->get_point(0), edge->get_point(1), t));
                assert(e);
                return e;
            }
        case et_quad:
            {
                vec2 qpts[3] = { edge->get_point(0), edge->get_point(1), edge->get_point(2) }, spts[5];
                split_quad_bezier(spts, qpts, t);
                auto e = create_quad_edge(spts[0], spts[1], spts[2]);
                assert(e);
                return e;
            }
        case et_cubic:
            {
                vec2 cpts[4] = { edge->get_point(0), edge->get_point(1), edge->get_point(2), edge->get_point(3) }, spts[7];
                split_cubic_bezier(spts, cpts, t);
                auto e = create_cubic_edge(spts[0], spts[1], spts[2], spts[3]);
                assert(e);
                return e;
            }
        }
        return nullptr;
    }
    clip_edge* split_back_part(clip_edge* edge, float t)
    {
        assert(edge);
        switch(edge->get_type())
        {
        case et_linear:
            {
                auto e = create_linear_edge(vec2().lerp(edge->get_point(0), edge->get_point(1), t), edge->get_point(1));
                assert(e);
                return e;
            }
        case et_quad:
            {
                vec2 qpts[3] = { edge->get_point(0), edge->get_point(1), edge->get_point(2) }, spts[5];
                split_quad_bezier(spts, qpts, t);
                auto e = create_quad_edge(spts[2], spts[3], spts[4]);
                assert(e);
                return e;
            }
        case et_cubic:
            {
                vec2 cpts[4] = { edge->get_point(0), edge->get_point(1), edge->get_point(2), edge->get_point(3) }, spts[7];
                split_cubic_bezier(spts, cpts, t);
                auto e = create_cubic_edge(spts[3], spts[4], spts[5], spts[6]);
                assert(e);
                return e;
            }
        }
        return nullptr;
    }
    clip_edge* split_middle_part(clip_edge* edge, float t1, float t2)
    {
        assert(edge);
        assert(t1 < t2);
        switch(edge->get_type())
        {
        case et_linear:
            {
                auto e = create_linear_edge(vec2().lerp(edge->get_point(0), edge->get_point(1), t1), vec2().lerp(edge->get_point(0), edge->get_point(1), t2));
                assert(e);
                return e;
            }
        case et_quad:
            {
                vec2 qpts[3] = { edge->get_point(0), edge->get_point(1), edge->get_point(2) }, spts[7];
                split_quad_bezier(spts, qpts, t1, t2);
                auto e = create_quad_edge(spts[2], spts[3], spts[4]);
                assert(e);
                return e;
            }
        case et_cubic:
            {
                vec2 cpts[4] = { edge->get_point(0), edge->get_point(1), edge->get_point(2), edge->get_point(3) }, spts[10];
                split_cubic_bezier(spts, cpts, t1, t2);
                auto e = create_cubic_edge(spts[3], spts[4], spts[5], spts[6]);
                assert(e);
                return e;
            }
        }
        return nullptr;
    }
};

static float correct_intersection(vec2& correctpt, const vec2& pt, const clip_edge* e)
{
    assert(e);
    switch(e->get_type())
    {
    case et_linear:
        {
            float t = linear_reparameterize(e->get_point(0), e->get_point(1), pt);
            correctpt.lerp(e->get_point(0), e->get_point(1), t);
            return t;
        }
    case et_quad:
        {
            vec3 para[2];
            get_quad_parameter_equation(para, e->get_point(0), e->get_point(1), e->get_point(2));
            float t = best_quad_reparameterize(para, pt);
            eval_quad(correctpt, para, t);
            return t;
        }
    case et_cubic:
        {
            vec4 para[2];
            get_cubic_parameter_equation(para, e->get_point(0), e->get_point(1), e->get_point(2), e->get_point(3));
            float t = best_cubic_reparameterize(para, pt);
            eval_cubic(correctpt, para, t);
            return t;
        }
    }
    assert(!"error.");
    return 0.f;
}

static void clip_zfill(IntPoint& e1bot, IntPoint& e1top, IntPoint& e2bot, IntPoint& e2top, IntPoint& pt)
{
    auto e1botp = get_clip_point(e1bot);
    auto e1topp = get_clip_point(e1top);
    auto e2botp = get_clip_point(e2bot);
    auto e2topp = get_clip_point(e2top);
    auto e1 = determine_edge(e1botp, e1topp);
    auto e2 = determine_edge(e2botp, e2topp);
    if(!e1 || !e2) {
        assert(!"bad intersection.");
        return;
    }
    vec2 p = cvt_point(pt);
    vec2 fixp1, fixp2;
    float t1 = correct_intersection(fixp1, p, e1);
    float t2 = correct_intersection(fixp2, p, e2);
    vec2 fixp;
    fixp.add(fixp1, fixp2).scale(0.5f);
    assert(__current_clipper);
    auto ip = __current_clipper->create_intersect_point(fixp);
    assert(ip);
    pt = cvt_AJ_point(fixp, ip);
    ip->add_intersect_info(e1, t1);
    ip->add_intersect_info(e2, t2);
}

static void clip_simplify(painter_linestrips& lss, Paths& input)
{
    Paths out;
    SimplifyPolygons(input, out);
    convert_to_polygons(lss, out);
}

void clip_simplify(painter_linestrips& lss, const painter_linestrip& input)
{
    Paths paths;
    convert_to_clipper_paths(paths, input);
    clip_simplify(lss, paths);
}

void clip_simplify(painter_linestrips& lss, const painter_linestrips& input)
{
    Paths paths;
    convert_to_clipper_paths(paths, input);
    clip_simplify(lss, paths);
}

static void add_point_as_if_needed(painter_linestrip& ls, clip_point_attr& attrmap, const vec2& p, uint m, clip_point_tag tag)
{
    ls.add_point(p);
    if(m & tag) {
        auto r = attrmap.try_emplace(p, 0).first;
        r->second |= tag;
    }
}

void clip_remapping_points(painter_linestrips& output, clip_point_attr& attrmap, const painter_path& input, uint attr_selector, float step_len)
{
    painter_linestrip* pc = nullptr;
    const painter_path::node* last = nullptr;
    int c = input.size();
    for(int i = 0; i < c; i ++) {
        const painter_path::node* n = input.get_node(i);
        assert(n);
        switch(n->get_tag())
        {
        case painter_path::pt_moveto:
            {
                output.push_back(painter_linestrip());
                if(pc)  pc->finish();
                pc = &output.back();
                add_point_as_if_needed(*pc, attrmap, trunc_point(n->get_point()), attr_selector, cpt_corner);
                break;
            }
        case painter_path::pt_lineto:
            {
                assert(pc && last);
                add_point_as_if_needed(*pc, attrmap, trunc_point(n->get_point()), attr_selector, cpt_corner);
                break;
            }
        case painter_path::pt_quadto:
            {
                static_cast_as(const painter_path::quad_to_node*, qnode, n);
                const vec2& p0 = last->get_point();
                const vec2& p1 = qnode->get_control();
                const vec2& p2 = qnode->get_point();
                int step = get_interpolate_step(p0, p1, p2, step_len);
                int interstep = step - 1;
                if(interstep > 0) {
                    float chord, t;
                    chord = t = 1.f / interstep;
                    vec3 para[2];
                    get_quad_parameter_equation(para, p0, p1, p2);
                    vec2 p;
                    for(int j = 1; j < interstep; j ++, t += chord) {
                        eval_quad(p, para, t);
                        add_point_as_if_needed(*pc, attrmap, trunc_point(p), attr_selector, cpt_interpolate);
                    }
                }
                add_point_as_if_needed(*pc, attrmap, trunc_point(p2), attr_selector, cpt_corner);
                break;
            }
        case painter_path::pt_cubicto:
            {
                static_cast_as(const painter_path::cubic_to_node*, cnode, n);
                const vec2& p0 = last->get_point();
                const vec2& p1 = cnode->get_control1();
                const vec2& p2 = cnode->get_control2();
                const vec2& p3 = cnode->get_point();
                int step = get_interpolate_step(p0, p1, p2, p3, step_len);
                int interstep = step - 1;
                if(interstep > 0) {
                    float chord, t;
                    chord = t = 1.f / interstep;
                    vec4 para[2];
                    get_cubic_parameter_equation(para, p0, p1, p2, p3);
                    vec2 p;
                    for(int j = 1; j < interstep; j ++, t += chord) {
                        eval_cubic(p, para, t);
                        add_point_as_if_needed(*pc, attrmap, trunc_point(p), attr_selector, cpt_interpolate);
                    }
                }
                add_point_as_if_needed(*pc, attrmap, trunc_point(p3), attr_selector, cpt_corner);
                break;
            }
        }
        last = n;
    }
    if(pc)  pc->finish();
}

static void clip_convert_path(painter_path& out, clip_result_const_iter i)
{
    assert(i);
    const painter_path& path = *i;
    out.add_path(path);
    for(auto j = i.child(); j.is_valid(); j = j.next())
        clip_convert_path(out, j);
}

void clip_convert(painter_path& path, const clip_result& result)
{
    if(result.is_valid())
        clip_convert_path(path, result.get_root());
}

void clip_simplify(clip_result& output, const painter_path& input)
{
    clipper c;
    c.add_path(input, ptSubject);
    c.simplify(output);
}

void clip_union(clip_result& output, const painter_path& subjects, const painter_path& clips)
{
    clipper c;
    c.add_path(subjects, ptSubject);
    c.add_path(clips, ptClip);
    c.do_union(output);
}

void clip_intersect(clip_result& output, const painter_path& subjects, const painter_path& clips)
{
    clipper c;
    c.add_path(subjects, ptSubject);
    c.add_path(clips, ptClip);
    c.do_intersect(output);
}

void clip_substract(clip_result& output, const painter_path& subjects, const painter_path& clips)
{
    clipper c;
    c.add_path(subjects, ptSubject);
    c.add_path(clips, ptClip);
    c.do_substract(output);
}

void clip_exclude(clip_result& output, const painter_path& subjects, const painter_path& clips)
{
    clipper c;
    c.add_path(subjects, ptSubject);
    c.add_path(clips, ptClip);
    c.do_exclude(output);
}

__ariel_end__
