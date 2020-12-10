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

#ifndef mesh_7c5c4ae5_0f3e_4a2d_936e_0447239ac3a7_h
#define mesh_7c5c4ae5_0f3e_4a2d_936e_0447239ac3a7_h

#include <gslib/type.h>
#include <gslib/std.h>
#include <ariel/type.h>

__ariel_begin__

struct mesh_uv_set;
struct mesh_tangent_set;

typedef vector<vec3> mesh_points;
typedef vector<vec3> mesh_normals;
typedef vector<vec3> mesh_tangents;
typedef vector<vec3> mesh_colors;
typedef vector<vec2> mesh_uv_coords;
typedef vector<int32> mesh_faces;
typedef vector<mesh_uv_set> mesh_uv_sets;
typedef vector<mesh_tangent_set> mesh_tangent_sets;

struct mesh_uv_set
{
    string              name;
    mesh_uv_coords      coords;
};

struct mesh_tangent_set
{
    string              name;
    mesh_tangents       tangents;
};

class mesh:
    public res_node
{
    friend class mesh_io;

public:
    mesh();
    virtual ~mesh() {}
    virtual res_type get_type() const override { return res_mesh; }
    bool has_indices() const { return !_faces.empty(); }
    bool has_points() const { return !_points.empty(); }
    bool has_normals() const { return !_normals.empty(); }
    bool has_colors() const { return !_vcolors.empty(); }
    bool has_uvs() const { return !_uvsets.empty(); }
    bool has_tangents() const { return !_tangentsets.empty(); }
    int32 num_uv_sets() const { return (int32)_uvsets.size(); }
    int32 num_tangent_sets() const { return (int32)_tangentsets.size(); }
    int32 num_points() const { return (int32)_points.size(); }
    int32 num_indices() const { return (int32)_faces.size(); }
    const mesh_points& get_const_points() const { return _points; }
    const mesh_faces& get_const_indices() const { return _faces; }
    const mesh_normals& get_const_normals() const { return _normals; }
    const mesh_colors& get_const_colors() const { return _vcolors; }

protected:
    mesh_points         _points;
    mesh_normals        _normals;
    mesh_colors         _vcolors;
    mesh_uv_sets        _uvsets;
    mesh_tangent_sets   _tangentsets;
    mesh_faces          _faces;
    matrix              _transforms;

public:
    /*
     * Caution:
     * If there were splited vertices in the mesh, the normals & tangents could be incorrect,
     * in such situations, you should always trust the data from exporter.
     */
    void calc_normals();
};

__ariel_end__

#endif
