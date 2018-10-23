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

#ifndef mesh_7c5c4ae5_0f3e_4a2d_936e_0447239ac3a7_h
#define mesh_7c5c4ae5_0f3e_4a2d_936e_0447239ac3a7_h

#include <ariel/config.h>
#include <gslib/type.h>
#include <gslib/std.h>

__ariel_begin__

#define mesh_invalid_index  -1
#define mesh_make_edge(start_point_index, end_point_index) ((((uint64)start_point_index) << 32) | end_point_index)
#define mesh_start_query_index(index) mesh_make_edge(index, mesh_invalid_index)
#define mesh_end_query_index(index) mesh_make_edge(mesh_invalid_index, index)

class mesh_point_data;
typedef uint32 mesh_point_index;
typedef uint64 mesh_edge;
typedef vector<mesh_point_data*> mesh_point_table;
typedef vector<mesh_point_index> mesh_triangles;
typedef unordered_multimap<mesh_point_index, mesh_point_index> mesh_sp_table;
typedef unordered_multimap<mesh_edge, mesh_point_index> mesh_digraph;
typedef unordered_map<mesh_edge, mesh_point_index> mesh_ef_table;

class mesh_point_data
{
public:
    mesh_point_data();
    virtual ~mesh_point_data();
    void set_index(mesh_point_index i) { _index = i; }
    mesh_point_index get_index() const { return _index; }
    void set_point(const vec3& p) { _point = p; }
    const vec3& get_point() const { return _point; }
    void set_normal(const vec3& n) { _normal = n; }
    const vec3& get_normal() const { return _normal; }
    void set_user_data(void* p) { _userdata = p; }
    void* get_user_data() const { return _userdata; }
    void acc_normal(const vec3& n) { _normal += n; }
    void normalize_normal() { _normal.normalize(); }

protected:
    mesh_point_index    _index;
    vec3                _point;
    vec3                _normal;
    void*               _userdata;
};

class mesh
{
public:
    mesh();
    virtual ~mesh();
    int load_from_text(const string& src, int start);
    const mesh_point_table& get_point_table() const { return _point_table; }
    int get_point_count() const { return (int)_point_table.size(); }
    mesh_point_data* get_point_data(mesh_point_index i) const { return _point_table.at(i); }

protected:
    mesh_point_table    _point_table;
    mesh_triangles      _triangles;
    matrix              _localmat;      /* todo: to be removed. */

protected:
    int load_local_section_from_text(const string& src, int start);
    int load_point_section_from_text(const string& src, int start);
    int load_face_section_from_text(const string& src, int start);
    void arrange_point_indices();
    void destroy_point_table();
    void calculate_normals();
};

class editable_mesh:
    public mesh
{
protected:
    mesh_sp_table       _sp_table;
    mesh_digraph        _digraph;
    mesh_ef_table       _ef_table;
};

/* todo: to be removed */
class __gs_novtable mesh_loader_for_test abstract
{
public:
    virtual ~mesh_loader_for_test() {}
    virtual bool load_from_text(const string& src);
    virtual void on_add_mesh(mesh* ptr) { delete ptr; }
};

__ariel_end__

#endif