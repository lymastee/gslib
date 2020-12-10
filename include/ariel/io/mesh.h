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

#ifndef mesh_50c7db6c_bce9_4a36_bd74_1d9b9c88dfa1_h
#define mesh_50c7db6c_bce9_4a36_bd74_1d9b9c88dfa1_h

#include <ariel/mesh.h>

__ariel_begin__

class mesh_io
{
public:
    mesh_io(mesh& m);
    int32 load_from_text(const string& src, int32 start);
    bool load_from_binary(io_binary_stream& bst);
    int32 confirm_and_load_from_text(const string& src, int32 start);
    bool confirm_and_load_from_binary(io_binary_stream& bst);

protected:
    mesh&                   _mesh;

protected:
    int32 load_point_section_from_text(const string& src, int32 start);
    int32 load_normal_section_from_text(const string& src, int32 start);
    int32 load_color_section_from_text(const string& src, int32 start);
    int32 load_uv_section_from_text(const string& src, int32 start);
    int32 load_tangent_section_from_text(const string& src, int32 start);
    int32 load_face_section_from_text(const string& src, int32 start);
    int32 load_transform_section_from_text(const string& src, int32 start);
    bool load_point_section_from_binary(io_binary_stream& bst);
    bool load_normal_section_from_binary(io_binary_stream& bst);
    bool load_color_section_from_binary(io_binary_stream& bst);
    bool load_uv_section_from_binary(io_binary_stream& bst);
    bool load_tangent_section_from_binary(io_binary_stream& bst);
    bool load_face_section_from_binary(io_binary_stream& bst);
    bool load_transform_section_from_binary(io_binary_stream& bst);
};

__ariel_end__

#endif
