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

#include <ariel/clip3.h>

#undef min
#undef max

#include <clipper/clipper.hpp>
#include <clipper/clipper.cpp>

__ariel_begin__

using namespace ClipperLib;

#define int_scale_ratio     1000.f

static IntPoint convert_to_clipper_point(const vec2& pt)
{
    return IntPoint(round(pt.x * int_scale_ratio), round(pt.y * int_scale_ratio));
}

static vec2 convert_to_point(const IntPoint& pt)
{
    return vec2((float)pt.X / int_scale_ratio, (float)pt.Y / int_scale_ratio);
}

static void convert_to_polygons(painter_linestrips& polygons, const Paths& out)
{
    for(auto i = out.begin(); i != out.end(); ++i) {
        if(!i->empty()) {
            polygons.push_back(painter_linestrip());
            painter_linestrip& ls = polygons.back();
            for(int j = (int)i->size() - 1; j >= 0; j--)
                ls.add_point(convert_to_point(i->at(j)));
        }
    }
}

static void convert_to_paths(painter_paths& paths, const Paths& out)
{
    for(auto i = out.begin(); i != out.end(); ++i) {
        if(!i->empty()) {
            paths.push_back(painter_path());
            painter_path& path = paths.back();
            path.move_to(convert_to_point(i->front()));
            for(int j = (int)i->size() - 1; j > 0; j--)
                path.line_to(convert_to_point(i->at(j)));
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
            cp.push_back(convert_to_clipper_point(ls.get_point(i)));
    }
}

static void convert_to_clipper_paths(Paths& paths, const painter_linestrips& lss)
{
    for(auto i = lss.begin(); i != lss.end(); ++i)
        convert_to_clipper_paths(paths, *i);
}

class clip_edge;
class clip_point;
class clipper;

typedef vector<clip_edge*> clip_edges;
typedef vector<clip_point*> clip_points;

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

typedef clip_edge_tpl<et_linear, 2> clip_linear_edge;
typedef clip_edge_tpl<et_quad, 3> clip_quad_edge;
typedef clip_edge_tpl<et_cubic, 4> clip_cubic_edge;

typedef map<clip_edge*, float> clip_intersect_info_map;

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

public:
    virtual type get_type() const override { return pt_intersect_point; }

private:
    intersect_info_map  _itrs_map;

public:

};

static clipper*  __current_clipper = nullptr;

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
        _edges.clear();
        _points.clear();
        __current_clipper = nullptr;
    }
    void add_path(const painter_path& path)
    {
        for(int i = 0; i < path.size();) {
            painter_path sp;
            i = path.get_sub_path(sp, i);
            add_sub_path(sp);
        }
    }
    void add_sub_path(const painter_path& path)
    {
    }

private:
    Clipper         _AJ_clipper;
    clip_edges      _edges;
    clip_points     _points;
};

static void clip_zfill(IntPoint& e1bot, IntPoint& e1top, IntPoint& e2bot, IntPoint& e2top, IntPoint& pt)
{
}

static void simplify_polygons(painter_linestrips& lss, Paths& input)
{
    Paths out;
    SimplifyPolygons(input, out);
    convert_to_polygons(lss, out);
}

void simplify_polygon(painter_linestrips& lss, const painter_linestrip& input)
{
    Paths paths;
    convert_to_clipper_paths(paths, input);
    simplify_polygons(lss, paths);
}

void simplify_polygons(painter_linestrips& lss, const painter_linestrips& input)
{
    Paths paths;
    convert_to_clipper_paths(paths, input);
    simplify_polygons(lss, paths);
}

void simplify_path(painter_paths& paths, const painter_path& input)
{
    linestrips lss;
    input.get_linestrips(lss);
    Paths cpaths;
    for(auto i = lss.begin(); i != lss.end(); ++ i) {
        Path cp;
        int s = i->get_size();
        i->tracing_segments();
        for(int j = 0; j < s; j ++) {
            auto p = i->get_point(j);
            cp.push_back(convert_to_clipper_point(p));
        }
        cpaths.push_back(cp);
    }
    Paths out;
    SimplifyPolygons(cpaths, out);
    convert_to_paths(paths, out);
    for(auto& p : paths)
        p.tracing_segments();
    __asm nop;
}

__ariel_end__
