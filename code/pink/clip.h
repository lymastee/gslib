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

#ifndef clip_84d62df0_3a5c_4f7c_81dc_20b38c3b391c_h
#define clip_84d62df0_3a5c_4f7c_81dc_20b38c3b391c_h

#include <gslib/std.h>
#include <gslib/tree.h>
#include <gslib/mclass.h>
#include <pink/utility.h>
#include <pink/raster.h>

__pink_begin__

class clip_joint;
class clip_line;
class clip_polygon;
class clip_patch;
typedef vector<clip_joint*> clip_joints;
typedef vector<clip_line*> clip_lines;
typedef vector<path_info*> clip_path_infos;
typedef vector<clip_polygon*> clip_polygons;

enum clip_joint_type
{
    ct_end_joint,
    ct_interpolate_joint,
    ct_intersect_joint,
    ct_pivot_joint,
    ct_mirror_joint,
    ct_final_joint,
};

class __gs_novtable clip_joint abstract
{
protected:
    clip_line*          _prev;
    clip_line*          _next;

public:
    clip_joint() { _prev = _next = 0; }
    virtual ~clip_joint() {}
    virtual clip_joint_type get_type() const = 0;
    virtual const vec2& get_point() const = 0;

public:
    clip_line* get_prev_line() const { return _prev; }
    clip_line* get_next_line() const { return _next; }
    clip_joint* get_prev_joint() const;
    clip_joint* get_next_joint() const;
    void set_prev_line(clip_line* line) { _prev = line; }
    void set_next_line(clip_line* line) { _next = line; }
    void reverse() { gs_swap(_prev, _next); }

protected:
    friend class clip_sweep_line_algorithm;
    clip_line* get_prev_above(float y) const;
    clip_line* get_next_above(float y) const;
    clip_line* get_prev_below(float y) const;
    clip_line* get_next_below(float y) const;
};

struct path_info;

class clip_end_joint:
    public clip_joint
{
protected:
    vec2                _point;
    path_info*          _info[2];

public:
    clip_end_joint(const vec2& pt);
    clip_joint_type get_type() const override { return ct_end_joint; }
    const vec2& get_point() const override { return _point; }
    void set_path_info(int i, path_info* pnf);
    path_info* get_path_info(int i) const { return _info[i]; }
};

class clip_interpolate_joint:
    public clip_joint
{
protected:
    vec2                _point;
    path_info*          _info;
    float               _ratio;

public:
    clip_interpolate_joint(const vec2& pt);
    clip_joint_type get_type() const override { return ct_interpolate_joint; }
    const vec2& get_point() const override { return _point; }
    void set_path_info(path_info* pnf) { _info = pnf; }
    path_info* get_path_info() const { return _info; }
    void set_ratio(float t) { _ratio = t; }
    float get_ratio() const { return _ratio; }
};

class clip_intersect_joint:
    public clip_joint
{
protected:
    vec2                _point;
    clip_joint*         _orient[2];
    clip_joint*         _cut[2];
    clip_intersect_joint* _symmetric;

public:
    clip_intersect_joint(const vec2& pt);
    clip_joint_type get_type() const override { return ct_intersect_joint; }
    const vec2& get_point() const override { return _point; }
    void set_orient(int i, clip_joint* j) { _orient[i] = j; }
    void set_cut(int i, clip_joint* j) { _cut[i] = j; }
    clip_joint* get_orient(int i) const { return _orient[i]; }
    clip_joint* get_cut(int i) const { return _cut[i]; }
    void set_symmetric(clip_intersect_joint* joint) { _symmetric = joint; }
    clip_intersect_joint* get_symmetric() const { return _symmetric; }
    path_info* get_path_info() const;
};

class clip_pivot_joint:
    public clip_joint
{

};

class clip_mirror_joint:
    public clip_joint
{
protected:
    clip_joint*         _mirror;

public:
    clip_mirror_joint() { _mirror = 0; }
    clip_joint_type get_type() const override { return ct_mirror_joint; }
    const vec2& get_point() const override { return _mirror->get_point(); }
    clip_joint* get_mirror() const { return _mirror; }
    void set_mirror(clip_joint* joint) { _mirror = joint; }
};

class clip_final_joint:
    public clip_joint
{
protected:
    vec2                _point;
    clip_joint*         _mirror;

public:
    clip_final_joint() { _mirror = 0; }
    clip_joint_type get_type() const override { return ct_final_joint; }
    const vec2& get_point() const override { return _point; }
    void set_point(const vec2& pt) { _point = pt; }
    bool is_control_point() const { return !_mirror; }
    clip_joint* get_mirror() const { return _mirror; }
    void set_mirror(clip_joint* joint) { _mirror = joint; }
};

class clip_line
{
protected:
    clip_joint*         _joint[2];

public:
    clip_line() { _joint[0] = _joint[1] = 0; }
    const vec2& get_point(int i) const { return _joint[i]->get_point(); }
    void set_joint(int i, clip_joint* joint) { _joint[i] = joint; }
    clip_joint* get_joint(int i) const { return _joint[i]; }
    clip_joint* get_another_joint(const clip_joint* joint) const;
    clip_line* get_prev_line() const;
    clip_line* get_next_line() const;
    void reverse() { gs_swap(_joint[0], _joint[1]); }
    void tracing() const;
};

class clip_polygon
{
protected:
    clip_joints         _joint_holdings;
    clip_lines          _line_holdings;
    clip_path_infos     _pnf_holdings;
    clip_line*          _line_start;

public:
    clip_polygon() { _line_start = 0; }
    virtual ~clip_polygon();
    virtual bool is_patch() const { return false; }

public:
    bool is_valid() const { return _line_start != 0; }
    void set_line_start(clip_line* line) { _line_start = line; }
    clip_line* get_line_start() const { return _line_start; }
    clip_joints& get_joints() { return _joint_holdings; }
    int create(const painter_path& path, int start);
    void convert_from(clip_patch& patch);
    void reverse_direction();
    void tracing() const;
    void trace_segments() const;
    void trace_final() const;

protected:
    int create_path_infos(const painter_path& path, int start);
    void create_polygon();
    clip_end_joint* create_end_joint(path_info* pnf1, path_info* pnf2);
    clip_interpolate_joint* create_interpolate_joint(path_info* pnf, const vec2& p, float t);
    clip_line* create_line(clip_joint* joint1, clip_joint* joint2);
    clip_end_joint* create_segment(clip_joint* joint1, path_info* pnf1, path_info* pnf2, clip_joint* joint2 = 0);
    clip_mirror_joint* create_mirror_joint(clip_joint* joint);
    clip_final_joint* create_final_joint(clip_joint* joint);
    void adopt(clip_polygon& poly);

public:
    friend class clip_sweep_line_algorithm;
    friend class clip_assembler;
    friend class clip_assembler_exclude;
};

class clip_sweep_joint;
class clip_sweep_line;
class clip_sweep_relay;
struct clip_sweeper;
typedef clip_sweep_relay clip_sweep_key;
typedef list<clip_sweep_joint*> clip_sweep_joints;
typedef list<clip_sweep_line*> clip_sweep_lines;
typedef list<clip_sweeper> clip_sweepers;

enum clip_sweep_tag
{
    cst_relay,
    cst_endpoint,
};

class __gs_novtable clip_sweep_joint abstract
{
public:
    virtual ~clip_sweep_joint() {}
    virtual clip_sweep_tag get_tag() const = 0;
    virtual const vec2& get_point() const = 0;
};

class clip_sweep_relay:
    public clip_sweep_joint
{
protected:
    clip_line*          _line;
    vec2                _point;

public:
    clip_sweep_relay() { _line = 0; }
    clip_sweep_relay(const vec2& p) { _point = p; }
    clip_sweep_tag get_tag() const override { return cst_relay; }
    const vec2& get_point() const override { return _point; }
    clip_line* get_line() const { return _line; }
    void set_line(clip_line* l) { _line = l; }
    void set_point(const vec2& p) { _point = p; }
};

class clip_sweep_endpoint:
    public clip_sweep_joint
{
protected:
    clip_joint*         _joint;

public:
    clip_sweep_endpoint() { _joint = 0; }
    clip_sweep_endpoint(clip_joint* j) { _joint = j; }
    clip_sweep_tag get_tag() const override { return cst_endpoint; }
    const vec2& get_point() const override { return _joint->get_point(); }
    clip_joint* get_joint() const { return _joint; }
};

template<class _arg1, class _ret>
using clip_binary_func = std::binary_function<_arg1, _arg1, _ret>;

template<class _joint>
struct clip_order_x_ascend:
    public clip_binary_func<_joint, bool>
{
    typedef _joint joint;
    bool operator()(_joint* a, _joint* b) const
    {
        assert(a && b);
        return a->get_point().x - b->get_point().x < -1e-5f;
    }
};

template<class _joint>
struct clip_order_y_ascend:
    public clip_binary_func<_joint, bool>
{
    typedef _joint joint;
    bool operator()(_joint* a, _joint* b) const
    {
        assert(a && b);
        return a->get_point().y - b->get_point().y < -1e-5f;
    }
};

template<class _joint>
struct clip_order_x_descend:
    public clip_binary_func<_joint, bool>
{
    typedef _joint joint;
    bool operator()(_joint* a, _joint* b) const
    {
        assert(a && b);
        return a->get_point().x - b->get_point().x > 1e-5f;
    }
};

template<class _joint>
struct clip_order_y_descend:
    public clip_binary_func<_joint, bool>
{
    typedef _joint joint;
    bool operator()(_joint* a, _joint* b) const
    {
        assert(a && b);
        return a->get_point().y - b->get_point().y > 1e-5f;
    }
};

template<class _order>
using clip_joint_table = multiset<typename _order::joint*, _order>;

typedef clip_joint_table<clip_order_y_ascend<clip_joint> > alg_joint_table;
typedef clip_joint_table<clip_order_x_ascend<clip_sweep_joint> > sweep_joint_table;

class clip_sweep_line
{
protected:
    sweep_joint_table   _sorted_by_x;
    clip_sweep_joints   _joint_holdings;
    float               _y;

public:
    clip_sweep_line() {}
    clip_sweep_line(float y) { _y = y; }
    ~clip_sweep_line();
    void set_y(float y) { _y = y; }
    float get_y() const { return _y; }
    sweep_joint_table& get_sorted_joints() { return _sorted_by_x; }
    const sweep_joint_table& const_sorted_joints() const { return _sorted_by_x; }
    void add_joint(clip_sweep_joint* joint);
    clip_sweep_joint* create_joint(clip_joint* joint);
    clip_sweep_joint* create_joint(clip_line* line, float y);
    clip_sweep_joint* ensure_unique_create(clip_joint* joint);
};

struct clip_sweeper
{
    clip_sweep_joint*   joint;
    clip_line*          line;

public:
    clip_sweeper() {}
    clip_sweeper(clip_sweep_joint* j, clip_line* l) { joint = j, line = l; }
    const vec2& get_point() const { return joint->get_point(); }
    const vec2& get_another_point() const;
    void tracing() const;
};

class clip_patch:
    public clip_polygon
{
public:
    clip_patch() { _line_end = 0; }
    bool is_patch() const override { return true; }
    void set_line_end(clip_line* line) { _line_end = line; }
    clip_line* get_line_end() const { return _line_end; }
    clip_mirror_joint* create_assumed_mj(const clip_sweeper& sweeper);
    clip_line* fix_line_front(clip_line* line, clip_mirror_joint* mj);
    clip_line* fix_line_back(clip_line* line, clip_mirror_joint* mj);
    void set_start_point(const clip_sweeper& pt) { _start_point = pt; }
    void set_end_point(const clip_sweeper& pt) { _end_point = pt; }
    clip_sweeper& get_start_point() { return _start_point; }
    clip_sweeper& get_end_point() { return _end_point; }
    const clip_sweeper& const_start_point() const { return _start_point; }
    const clip_sweeper& const_end_point() const { return _end_point; }
    void reverse_direction();
    void finish_patch();
    void tracing() const;

protected:
    clip_line*          _line_end;
    clip_sweeper        _start_point;
    clip_sweeper        _end_point;
};

/*
 * Notice about the result tree:
 * root node could be empty, in case of the multi top entries.
 * after the output procedure was finished, the empty root case won't be fixed.
 */
typedef _treenode_wrapper<clip_polygon> clip_result_wrapper;
typedef tree<clip_polygon, clip_result_wrapper> clip_result;
typedef clip_result::iterator clip_result_iter;

enum clip_polygon_tag
{
    cp_union,
    cp_intersect,
    cp_exclude,
};

enum clip_assembly_node_tag
{
    ant_sweeper,
    ant_point,
    ant_patch,
};

class clip_assembly_sweeper_node
{
public:
    clip_assembly_sweeper_node() { _up = _down = 0; }
    clip_sweeper* get_up_sweeper() const { return _up; }
    clip_sweeper* get_down_sweeper() const { return _down; }
    void set_up_sweeper(clip_sweeper* p) { _up = p; }
    void set_down_sweeper(clip_sweeper* p) { _down = p; }

protected:
    clip_sweeper*       _up;
    clip_sweeper*       _down;
};

class clip_assembly_point_node
{
public:
    clip_assembly_point_node() { _sweeper = 0; }
    clip_result_iter get_patch_iter() const { return _iter; }
    void set_patch_iter(clip_result_iter i) { _iter = i; }
    clip_sweeper* get_sweeper() const { return _sweeper; }
    void set_sweeper(clip_sweeper* p) { _sweeper = p; }

protected:
    clip_result_iter    _iter;
    clip_sweeper*       _sweeper;
};

class clip_assembly_patch_node
{
public:
    clip_assembly_patch_node() {}
    clip_result_iter get_patch_iter() const { return _iter; }
    void set_patch_iter(clip_result_iter i) { _iter = i; }
    const vec2& get_point() const { return _point; }
    void set_point(const vec2& pt) { _point = pt; }

protected:
    clip_result_iter    _iter;
    vec2                _point;
};

typedef multi_class_3<clip_assembly_node_tag, clip_assembly_sweeper_node, clip_assembly_point_node, clip_assembly_patch_node> clip_assembly_node;
typedef list<clip_assembly_node> clip_assembly_nodes;

class clip_assembler
{
public:
    typedef clip_result::iterator iterator;
    typedef clip_result::const_iterator const_iterator;
    typedef clip_sweepers::iterator swpiter;
    typedef clip_sweepers::const_iterator const_swpiter;
    typedef stack<iterator> iterator_stack;

protected:
    clip_result&        _result;
    clip_sweepers       _upside;
    clip_sweepers       _downside;
    clip_assembly_nodes _nodes;
    iterator_stack      _iter_st;
    iterator            _last_sib;

public:
    clip_assembler(clip_result& result): _result(result) {}

protected:
    void install_sweepers(clip_sweep_line* line1, clip_sweep_line* line2);
    void prepare_assembly_nodes();
    iterator close_patch(iterator i, const clip_assembly_sweeper_node& node1, const clip_assembly_sweeper_node& node2);
    iterator close_patch(iterator i, iterator j);
    void merge_patch(const clip_assembly_point_node& node1, const clip_assembly_point_node& node2) { merge_patch_parallel(node1.get_patch_iter(), node2.get_patch_iter(), *node1.get_sweeper(), *node2.get_sweeper()); }
    void merge_patch_parallel(iterator piter1, iterator piter2, clip_sweeper& sweeper1, clip_sweeper& sweeper2);
    void merge_patch_head(iterator piter1, iterator piter2, clip_sweeper& sweeper1, clip_sweeper& sweeper2);
    void merge_patch_tail(iterator piter1, iterator piter2, clip_sweeper& sweeper1, clip_sweeper& sweeper2);
    void split_patch(iterator piter, clip_joint* sp1, clip_joint* sp2);
    bool try_split_patch(iterator piter);
    static void create_patch(clip_patch& patch, const clip_sweeper& up1, const clip_sweeper& up2, const clip_sweeper& down1, const clip_sweeper& down2);
    static void proceed_patch(clip_patch& patch, const clip_assembly_sweeper_node& node1, const clip_assembly_sweeper_node& node2);
};

class clip_assembler_exclude:
    public clip_assembler
{
public:
    clip_assembler_exclude(clip_result& result);
    void proceed(clip_sweep_lines& sweeplines);

protected:
    void proceed(clip_sweep_line* line1, clip_sweep_line* line2);
    void proceed_patches();
    void proceed_sweepers();
    void finish_proceed_patches(iterator p);
    void finish_proceed_sub_patches(iterator p);
};

// class clip_assembler_union;
// class clip_assembler_intersect;

typedef unordered_map<path_info*, curve_spliter*> curve_spliter_map;
typedef unordered_map<clip_joint*, vec2> clip_fixed_points;

class clip_sweep_line_algorithm
{
public:
    typedef clip_sweep_lines::iterator iterator;
    typedef clip_sweep_lines::const_iterator const_iterator;

public:
    clip_sweep_line_algorithm();
    ~clip_sweep_line_algorithm() { destroy(); }
    void destroy();
    void add_polygon(clip_polygon* polygon);
    void proceed();
    void output(clip_result& result);
    void set_clip_tag(clip_polygon_tag tag) { _clip_tag = tag; }
    clip_polygon_tag get_clip_tag() const { return _clip_tag; }
    void trace_sweep_lines() const;
    void trace_sweep_line(const clip_sweep_line* line) const;
    void trace_sweep_lines(const mat3& m) const;
    void trace_sweep_line(const clip_sweep_line* line, const mat3& m) const;
    void trace_intersections() const;

protected:
    clip_polygon_tag    _clip_tag;
    clip_joints         _joint_holdings;
    clip_lines          _line_holdings;
    alg_joint_table     _sorted_by_y;
    clip_sweep_lines    _sweep_lines;
    clip_joints         _intersections;
    curve_spliter_map   _spliters;
    clip_fixed_points   _fixed_points;

protected:
    void prepare_sweep_lines();
    void proceed_overlapped_intersections();
    void proceed_intersections();
    void finish_intersections();
    clip_sweep_line* create_sweep_line(clip_joint* joint);
    void proceed_sweep_line(clip_sweep_line* last, clip_sweep_line* curr);
    void proceed_sweep_line(clip_sweep_line* line, clip_sweepers::iterator from, clip_sweepers::iterator to);
    void proceed_overlapped_intersection(clip_sweep_line* line);
    void proceed_intersection(iterator a, iterator b);
    void proceed_intersection(clip_sweep_line* line, clip_sweeper& sweeper1, clip_sweeper& sweeper2, iterator a, iterator b);
    iterator insert_sweep_line(iterator i, iterator j);
    void finish_intersection(clip_intersect_joint* joint);
    void output_exclude(clip_result& result);
    void output_union(clip_result& result);
    void output_intersect(clip_result& result);
    void destroy_spliters();
    void destroy_fixed_points();
    void create_spliters();
    void create_spliter(clip_intersect_joint* p, path_info* pnf1, path_info* pnf2);
    void replace_curves(clip_result& result);
    void replace_curves(clip_result& result, clip_result::iterator p);
    void replace_curves(clip_polygon& poly);
    void replace_curve(clip_polygon& poly, clip_joint*& joint1, clip_joint*& joint2);
};

extern void clip_test(const painter_path& path);
extern void clip_create_polygons(clip_polygons& polygons, const painter_path& path);    /* ensure that you've called painter_helper::close_sub_paths to deal with the path */
extern void clip_compile_path(painter_path& path, const clip_result& poly_result);
extern void clip_union(clip_result& poly_result, clip_polygons& polygons);
extern void clip_intersect(clip_result& poly_result, clip_polygons& polygons);
extern void clip_exclude(clip_result& poly_result, clip_polygons& polygons);

__pink_end__

#endif
