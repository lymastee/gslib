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

#include <gslib/error.h>
#include <ariel/io/utilities.h>
#include <ariel/io/mesh.h>

/*
 * Text format:
 *  Mesh:meshName {
 *      .Points {
 *          x y z(optional, index)
 *          ...
 *          }
 *      .Normals {
 *          x y z(optional, index)
 *          ...
 *          }
 *      .Colors {
 *          r g b(optional, index)
 *          ...
 *          }
 *      .UVs:uvSetName {
 *          u v(optional, index)
 *          ...
 *          }
 *      .Tangents:tangentsName {
 *          x y z(optional, index)
 *          ...
 *          }
 *      .Faces {
 *          x y z(optional, index)
 *          ...
 *          }
 *      .Transforms {
 *          m11 m12 m13 m14
 *          m21 m22 m23 m24
 *          m31 m32 m33 m34
 *          m41 m42 m43 m44
 *          }
 *      }
 */

/*
 * Binary format:
 *  #Mesh@[size in bytes]
 *  #Mesh$meshName@[size in bytes]
 *  #.Points@[size in bytes]...
 *  #.Normals@[size in bytes]...
 */

__ariel_begin__

mesh_io::mesh_io(mesh& m):
    _mesh(m)
{
}

int32 mesh_io::load_from_text(const string& src, int32 start)
{
    assert(start >= 0 && start < src.length());
    int32 next = io_enter_section(src, start, _t(':'));
    if(next == -1)
        next = start;
    else {
        string notation;
        next = io_read_notation(src, notation, start = next);
        if(io_bad_eof(src, next))
            return -1;
        _mesh.set_name(notation);
    }
    next = io_enter_section(src, start = next);
    if(io_bad_eof(src, next))
        return -1;
    next = io_skip_blank_charactors(src, start = next);
    if(io_bad_eof(src, next))
        return -1;
    while(src.at(next) != _t('}')) {
        string name;
        next = io_read_section_name(src, name, start = next);
        if(io_bad_eof(src, next))
            return -1;
        /* format start with .Points */
        if(name == _t(".Points")) {
            next = load_point_section_from_text(src, start = next);
            if(io_bad_eof(src, next))
                return -1;
        }
        /* format start with .Normals */
        else if(name == _t(".Normals")) {
            next = load_normal_section_from_text(src, start = next);
            if(io_bad_eof(src, next))
                return -1;
        }
        /* format start with .Colors */
        else if(name == _t(".Colors")) {
            next = load_color_section_from_text(src, start = next);
            if(io_bad_eof(src, next))
                return -1;
        }
        /* format start with .UVs */
        else if(name == _t(".UVs")) {
            next = load_uv_section_from_text(src, start = next);
            if(io_bad_eof(src, next))
                return -1;
        }
        /* format start with .Tangents */
        else if(name == _t(".Tangents")) {
            next = load_tangent_section_from_text(src, start = next);
            if(io_bad_eof(src, next))
                return -1;
        }
        /* format start with .Faces */
        else if(name == _t(".Faces")) {
            next = load_face_section_from_text(src, start = next);
            if(io_bad_eof(src, next))
                return -1;
        }
        /* format start with .Transforms */
        else if(name == _t(".Transform")) {
            next = load_transform_section_from_text(src, start = next);
            if(io_bad_eof(src, next))
                return -1;
        }
        else {
            set_error(_t("Warning: Unexpected section skipped.\n"));
            next = io_skip_section(src, start = next);
            if(io_bad_eof(src, next))
                return -1;
        }
        next = io_skip_blank_charactors(src, start = next);
        if(io_bad_eof(src, next))
            return -1;
    }
    assert(src.at(next) == _t('}'));
    return ++ next;
}

int32 mesh_io::load_point_section_from_text(const string& src, int32 start)
{
    assert(start >= 0 && start < src.length());
    mesh_points& pts = _mesh._points;
    assert(pts.empty());
    int32 next = io_enter_section(src, start);
    if(io_bad_eof(src, next))
        return -1;
    next = io_skip_blank_charactors(src, start = next);
    if(io_bad_eof(src, next))
        return -1;
    while(src.at(next) != _t('}')) {
        /* the format was: float float float[optional](index)\r\n */
        string line;
        next = io_read_line_of_section(src, line, start = next);
        if(io_bad_eof(src, next))
            return -1;
        float x, y, z;
        int c = strtool::sscanf(line.c_str(), _t("%f %f %f"), &x, &y, &z);
        if(c != 3) {
            set_error(_t("Bad format about line in section...\n"));
            return -1;
        }
        /* record points */
        pts.push_back(vec3(x, y, z));
        /* step on */
        next = io_skip_blank_charactors(src, start = next);
        if(io_bad_eof(src, next))
            return -1;
    }
    pts.shrink_to_fit();
    assert(src.at(next) == _t('}'));
    return ++ next;
}

int32 mesh_io::load_normal_section_from_text(const string& src, int32 start)
{
    assert(start >= 0 && start < src.length());
    if(_mesh.has_normals()) {
        set_error(_t("Warning: A normal set already exist in current mesh.\n"));
        return io_skip_section(src, start);
    }
    mesh_normals& normals = _mesh._normals;
    int32 next = io_enter_section(src, start);
    if(io_bad_eof(src, next))
        return -1;
    next = io_skip_blank_charactors(src, start = next);
    if(io_bad_eof(src, next))
        return -1;
    auto propersize = _mesh._points.size();
    if(propersize > 0)
        normals.reserve(propersize);
    while(src.at(next) != _t('}')) {
        string line;
        next = io_read_line_of_section(src, line, start = next);
        if(io_bad_eof(src, next))
            return -1;
        float x, y, z;
        int c = strtool::sscanf(line.c_str(), _t("%f %f %f"), &x, &y, &z);
        if(c != 3) {
            set_error(_t("Bad format about line in section...\n"));
            return -1;
        }
        normals.push_back(vec3(x, y, z));
        next = io_skip_blank_charactors(src, start = next);
        if(io_bad_eof(src, next))
            return -1;
    }
    normals.shrink_to_fit();
    assert(src.at(next) == _t('}'));
    return ++ next;
}

int32 mesh_io::load_color_section_from_text(const string& src, int32 start)
{
    assert(start >= 0 && start < src.length());
    if(_mesh.has_colors()) {
        set_error(_t("Warning: A vcolor set already exist in current mesh.\n"));
        return io_skip_section(src, start);
    }
    mesh_colors& colors = _mesh._vcolors;
    int32 next = io_enter_section(src, start);
    if(io_bad_eof(src, next))
        return -1;
    next = io_skip_blank_charactors(src, start = next);
    if(io_bad_eof(src, next))
        return -1;
    auto propersize = _mesh._points.size();
    if(propersize > 0)
        colors.reserve(propersize);
    while(src.at(next) != _t('}')) {
        string line;
        next = io_read_line_of_section(src, line, start = next);
        if(io_bad_eof(src, next))
            return -1;
        float r, g, b;
        int c = strtool::sscanf(line.c_str(), _t("%f %f %f"), &r, &g, &b);
        if(c != 3) {
            set_error(_t("Bad format about line in section...\n"));
            return -1;
        }
        colors.push_back(vec3(r, g, b));
        next = io_skip_blank_charactors(src, start = next);
        if(io_bad_eof(src, next))
            return -1;
    }
    colors.shrink_to_fit();
    assert(src.at(next) == _t('}'));
    return ++ next;
}

int32 mesh_io::load_uv_section_from_text(const string& src, int32 start)
{
    assert(start >= 0 && start < src.length());
    _mesh._uvsets.push_back(mesh_uv_set());
    mesh_uv_set& uvset = _mesh._uvsets.back();
    int32 next = io_enter_section(src, start, _t(':'));
    if(next == -1)
        next = start;
    else {
        next = io_read_notation(src, uvset.name, start = next);
        if(io_bad_eof(src, next))
            return -1;
    }
    next = io_enter_section(src, start = next);
    if(io_bad_eof(src, next))
        return -1;
    next = io_skip_blank_charactors(src, start = next);
    if(io_bad_eof(src, next))
        return -1;
    auto propersize = _mesh._points.size();
    if(propersize > 0)
        uvset.coords.reserve(propersize);
    while(src.at(next) != _t('}')) {
        string line;
        next = io_read_line_of_section(src, line, start = next);
        if(io_bad_eof(src, next))
            return -1;
        float u, v;
        int c = strtool::sscanf(line.c_str(), _t("%f %f"), &u, &v);
        if(c != 2) {
            set_error(_t("Bad format about line in section...\n"));
            return -1;
        }
        uvset.coords.push_back(vec2(u, v));
        next = io_skip_blank_charactors(src, start = next);
        if(io_bad_eof(src, next))
            return -1;
    }
    uvset.coords.shrink_to_fit();
    assert(src.at(next) == _t('}'));
    return ++ next;
}

int32 mesh_io::load_tangent_section_from_text(const string& src, int32 start)
{
    assert(start >= 0 && start < src.length());
    _mesh._tangentsets.push_back(mesh_tangent_set());
    mesh_tangent_set& tangentset = _mesh._tangentsets.back();
    int32 next = io_enter_section(src, start, _t(':'));
    if(next == -1)
        next = start;
    else {
        next = io_read_notation(src, tangentset.name, start = next);
        if(io_bad_eof(src, next))
            return -1;
    }
    next = io_enter_section(src, start = next);
    if(io_bad_eof(src, next))
        return -1;
    next = io_skip_blank_charactors(src, start = next);
    if(io_bad_eof(src, next))
        return -1;
    auto propersize = _mesh._points.size();
    if(propersize > 0)
        tangentset.tangents.reserve(propersize);
    while(src.at(next) != _t('}')) {
        string line;
        next = io_read_line_of_section(src, line, start = next);
        if(io_bad_eof(src, next))
            return -1;
        float x, y, z;
        int c = strtool::sscanf(line.c_str(), _t("%f %f %f"), &x, &y, &z);
        if(c != 3) {
            set_error(_t("Bad format about line in section...\n"));
            return -1;
        }
        tangentset.tangents.push_back(vec3(x, y, z));
        next = io_skip_blank_charactors(src, start = next);
        if(io_bad_eof(src, next))
            return -1;
    }
    tangentset.tangents.shrink_to_fit();
    assert(src.at(next) == _t('}'));
    return ++ next;
}

int32 mesh_io::load_face_section_from_text(const string& src, int32 start)
{
    assert(start >= 0 && start < src.length());
    mesh_faces& faces = _mesh._faces;
    assert(faces.empty());
    int32 next = io_enter_section(src, start);
    if(io_bad_eof(src, next))
        return -1;
    next = io_skip_blank_charactors(src, start = next);
    if(io_bad_eof(src, next))
        return -1;
    while(src.at(next) != _t('}')) {
        string line;
        next = io_read_line_of_section(src, line, start = next);
        if(io_bad_eof(src, next))
            return -1;
        int i, j, k;
        int c = strtool::sscanf(line.c_str(), _t("%d %d %d"), &i, &j, &k);
        if(c != 3) {
            set_error(_t("Bad format about line in section...\n"));
            return -1;
        }
        faces.push_back(i);
        faces.push_back(j);
        faces.push_back(k);
        next = io_skip_blank_charactors(src, start = next);
        if(io_bad_eof(src, next))
            return -1;
    }
    faces.shrink_to_fit();
    assert(src.at(next) == _t('}'));
    return ++ next;
}

int32 mesh_io::load_transform_section_from_text(const string& src, int32 start)
{
    assert(start >= 0 && start < src.length());
    int32 next = io_enter_section(src, start);
    if(io_bad_eof(src, next))
        return -1;
    next = io_skip_blank_charactors(src, start = next);
    if(io_bad_eof(src, next))
        return -1;
    vector<float> read_floats;
    read_floats.reserve(16);
    while(src.at(next) != _t('}')) {
        string line;
        next = io_read_line_of_section(src, line, start = next);
        if(io_bad_eof(src, next))
            return -1;
        float a, b, c, d;
        int ct = strtool::sscanf(line.c_str(), _t("%f %f %f %f"), &a, &b, &c, &d);
        if(ct != 4) {
            set_error(_t("Bad format about line in section...\n"));
            return -1;
        }
        read_floats.push_back(a);
        read_floats.push_back(b);
        read_floats.push_back(c);
        read_floats.push_back(d);
        next = io_skip_blank_charactors(src, start = next);
        if(io_bad_eof(src, next))
            return -1;
    }
    if(read_floats.size() != 16)
        return -1;
    matrix& m = _mesh._transforms;
    for(int i = 0, k = 0; i < 4; i ++) {
        for(int j = 0; j < 4; j ++, k ++)
            m.m[i][j] = read_floats.at(k);
    }
    assert(src.at(next) == _t('}'));
    return ++ next;
}

bool mesh_io::load_from_binary(io_binary_stream& bst)
{
    /* the cursor should now be at #Mesh'(here) */
    auto ct = bst.read_control_type();
    if(ct == io_binary_stream::ctl_notation) {
        string name;
        int32 nsz = bst.read_string(name, _t("@"));
        if(nsz <= 0) {
            set_error(_t("mesh_io: Bad binary format.\n"));
            return false;
        }
        _mesh.set_name(name);
        ct = bst.read_control_type();
    }
    if(ct == io_binary_stream::ctl_counter) {
        int32 size = (int32)bst.read_dword();
        bst.enter_section(size);
    }
    else {
        set_error(_t("mesh_io: Bad binary format.\n"));
        return false;
    }
    while(!bst.exit_section()) {
        ct = bst.read_control_type();
        if(ct != io_binary_stream::ctl_section) {
            set_error(_t("mesh_io: Unexpected control tag in binary stream.\n"));
            return false;
        }
        string name;
        int32 nsz = bst.read_string(name, _t("$@"));
        assert(nsz > 0);
        if(name == _t(".Points")) {
            if(!load_point_section_from_binary(bst))
                return false;
        }
        else if(name == _t(".Normals")) {
            if(!load_normal_section_from_binary(bst))
                return false;
        }
        else if(name == _t(".Colors")) {
            if(!load_color_section_from_binary(bst))
                return false;
        }
        else if(name == _t(".UVs")) {
            if(!load_uv_section_from_binary(bst))
                return false;
        }
        else if(name == _t(".Tangents")) {
            if(!load_tangent_section_from_binary(bst))
                return false;
        }
        else if(name == _t(".Faces")) {
            if(!load_face_section_from_binary(bst))
                return false;
        }
        else if(name == _t(".Transforms")) {
            if(!load_transform_section_from_binary(bst))
                return false;
        }
        else {
            set_error(_t("Warning: Unexpected section in Mesh.\n"));
            if(!bst.skip_next_section()) {
                set_error(_t("mesh_io: Skip unknown section failed.\n"));
                return false;
            }
        }
    }
    return true;
}

int32 mesh_io::confirm_and_load_from_text(const string& src, int32 start)
{
    int32 next = io_skip_blank_charactors(src, start);
    if(io_bad_eof(src, next))
        return start;
    string name;
    next = io_read_section_name(src, name, next);
    if(io_bad_eof(src, next))
        return start;
    if(name == _t("Mesh"))
        return load_from_text(src, next);
    return start;
}

bool mesh_io::confirm_and_load_from_binary(io_binary_stream& bst)
{
    auto sto = bst.current_dev_pos();
    auto ct = bst.read_control_type();
    if(ct != io_binary_stream::ctl_section) {
        bst.seek_to(sto);
        return false;
    }
    string name;
    if(bst.read_string(name, _t("$@")) <= 0 || name != _t("Mesh")) {
        bst.seek_to(sto);
        return false;
    }
    return load_from_binary(bst);
}

bool mesh_io::load_point_section_from_binary(io_binary_stream& bst)
{
    auto ct = bst.read_control_type();
    if(ct != io_binary_stream::ctl_counter) {
        set_error(_t("mesh_io: Load point section failed.\n"));
        return false;
    }
    int32 section_sz = (int32)bst.read_dword();
    if(section_sz <= 0) {
        set_error(_t("mesh_io: Bad size for point section.\n"));
        return false;
    }
    bst.enter_section(section_sz);
    mesh_points& pts = _mesh._points;
    assert(pts.empty());
    assert(section_sz % 4 == 0);
    int32 coord_cnt = section_sz / 4;
    float* coords = new float[coord_cnt];
    bst.read_field_to_buf(coords, section_sz);
    assert(coord_cnt % 3 == 0);
    int32 pt_cnt = coord_cnt / 3;
    for(int32 i = 0, j = 0; i < pt_cnt; i ++, j += 3)
        pts.push_back(vec3(coords[j], coords[j + 1], coords[j + 2]));
    delete [] coords;
    return bst.exit_section();
}

bool mesh_io::load_normal_section_from_binary(io_binary_stream& bst)
{
    if(_mesh.has_normals()) {
        set_error(_t("Warning: A normal set already exist in current mesh.\n"));
        return bst.skip_next_section();
    }
    auto ct = bst.read_control_type();
    if(ct != io_binary_stream::ctl_counter) {
        set_error(_t("mesh_io: Load normal section failed.\n"));
        return false;
    }
    int32 section_sz = (int32)bst.read_dword();
    if(section_sz <= 0) {
        set_error(_t("mesh_io: Bad size for normal section.\n"));
        return false;
    }
    bst.enter_section(section_sz);
    mesh_normals& normals = _mesh._normals;
    assert(normals.empty());
    assert(section_sz % 4 == 0);
    int32 coord_cnt = section_sz / 4;
    float* coords = new float[coord_cnt];
    bst.read_field_to_buf(coords, section_sz);
    assert(coord_cnt % 3 == 0);
    int32 pt_cnt = coord_cnt / 3;
    for(int32 i = 0, j = 0; i < pt_cnt; i ++, j += 3)
        normals.push_back(vec3(coords[j], coords[j + 1], coords[j + 2]));
    delete [] coords;
    return bst.exit_section();
}

bool mesh_io::load_color_section_from_binary(io_binary_stream& bst)
{
    if(_mesh.has_colors()) {
        set_error(_t("Warning: A color set already exist in current mesh.\n"));
        return bst.skip_next_section();
    }
    auto ct = bst.read_control_type();
    if(ct != io_binary_stream::ctl_counter) {
        set_error(_t("mesh_io: Load color section failed.\n"));
        return false;
    }
    int32 section_sz = (int32)bst.read_dword();
    if(section_sz <= 0) {
        set_error(_t("mesh_io: Bad size for color section.\n"));
        return false;
    }
    bst.enter_section(section_sz);
    mesh_colors& colors = _mesh._vcolors;
    assert(colors.empty());
    assert(section_sz % 4 == 0);
    int32 coord_cnt = section_sz / 4;
    float* coords = new float[coord_cnt];
    bst.read_field_to_buf(coords, section_sz);
    assert(coord_cnt % 3 == 0);
    int32 cr_cnt = coord_cnt / 3;
    for(int32 i = 0, j = 0; i < cr_cnt; i ++, j += 3)
        colors.push_back(vec3(coords[j], coords[j + 1], coords[j + 2]));
    delete [] coords;
    return bst.exit_section();
}

bool mesh_io::load_uv_section_from_binary(io_binary_stream& bst)
{
    _mesh._uvsets.push_back(mesh_uv_set());
    mesh_uv_set& uvset = _mesh._uvsets.back();
    auto ct = bst.read_control_type();
    if(ct == io_binary_stream::ctl_notation) {
        int32 nsz = bst.read_string(uvset.name, _t("@"));
        if(nsz <= 0) {
            set_error(_t("mesh_io: Bad name for uv section.\n"));
            return false;
        }
        ct = bst.read_control_type();
    }
    if(ct != io_binary_stream::ctl_counter) {
        set_error(_t("mesh_io: Bad format for uv section.\n"));
        return false;
    }
    int32 section_sz = (int32)bst.read_dword();
    if(section_sz <= 0) {
        set_error(_t("mesh_io: Bad size for uv section.\n"));
        return false;
    }
    bst.enter_section(section_sz);
    assert(section_sz % 4 == 0);
    int32 coord_cnt = section_sz / 4;
    float* coords = new float[coord_cnt];
    bst.read_field_to_buf(coords, section_sz);
    assert(coord_cnt % 2 == 0);
    int32 uv_cnt = coord_cnt / 2;
    for(int32 i = 0, j = 0; i < uv_cnt; i ++, j += 2)
        uvset.coords.push_back(vec2(coords[j], coords[j + 1]));
    delete [] coords;
    return bst.exit_section();
}

bool mesh_io::load_tangent_section_from_binary(io_binary_stream& bst)
{
    _mesh._tangentsets.push_back(mesh_tangent_set());
    mesh_tangent_set& tangentset = _mesh._tangentsets.back();
    auto ct = bst.read_control_type();
    if(ct == io_binary_stream::ctl_notation) {
        int32 nsz = bst.read_string(tangentset.name, _t("@"));
        if(nsz <= 0) {
            set_error(_t("mesh_io: Bad name for tangent section.\n"));
            return false;
        }
        ct = bst.read_control_type();
    }
    if(ct != io_binary_stream::ctl_counter) {
        set_error(_t("mesh_io: Bad format for tangent section.\n"));
        return false;
    }
    int32 section_sz = (int32)bst.read_dword();
    if(section_sz <= 0) {
        set_error(_t("mesh_io: Bad size for tangent section.\n"));
        return false;
    }
    bst.enter_section(section_sz);
    assert(section_sz % 4 == 0);
    int32 coord_cnt = section_sz / 4;
    float* coords = new float[coord_cnt];
    bst.read_field_to_buf(coords, section_sz);
    assert(coord_cnt % 3 == 0);
    int32 uv_cnt = coord_cnt / 3;
    for(int32 i = 0, j = 0; i < uv_cnt; i ++, j += 3)
        tangentset.tangents.push_back(vec3(coords[j], coords[j + 1], coords[j + 2]));
    delete[] coords;
    return bst.exit_section();
}

bool mesh_io::load_face_section_from_binary(io_binary_stream& bst)
{
    auto ct = bst.read_control_type();
    if(ct != io_binary_stream::ctl_counter) {
        set_error(_t("mesh_io: Bad format for face section.\n"));
        return false;
    }
    int32 section_sz = (int32)bst.read_dword();
    assert(section_sz % 4 == 0);
    bst.enter_section(section_sz);
    int32 id_sz = section_sz / 4;
    assert(id_sz % 3 == 0);
    mesh_faces& faces = _mesh._faces;
    faces.resize(id_sz);
    bst.read_field_to_buf(&faces.front(), section_sz);
    return bst.exit_section();
}

bool mesh_io::load_transform_section_from_binary(io_binary_stream& bst)
{
    auto ct = bst.read_control_type();
    if(ct != io_binary_stream::ctl_counter) {
        set_error(_t("mesh_io: Bad format for transform section.\n"));
        return false;
    }
    int32 section_sz = (int32)bst.read_dword();
    if(section_sz != 16 * 4) {
        set_error(_t("mesh_io: Bad format for transform section.\n"));
        return false;
    }
    bst.enter_section(section_sz);
    bst.read_field_to_buf(&_mesh._transforms._11, section_sz);
    return bst.exit_section();
}

__ariel_end__
