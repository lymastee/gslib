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

#ifndef smaa_2dfbd37d_c446_4c7d_be3a_e6f7a9cf8330_h
#define smaa_2dfbd37d_c446_4c7d_be3a_e6f7a9cf8330_h

#include <ariel/config.h>
#include <ariel/rendersys.h>

__ariel_begin__

struct smaa_configs
{
    vec4                    subsample_indices;
    vec4                    rt_metrics;
};

class smaa_render_target
{
public:
    smaa_render_target(rendersys* rsys, int w, int h, uint format);
    ~smaa_render_target();

protected:
    int                     _width, _height;
    render_texture2d*       _tex;
    render_target_view*     _rtv;
    shader_resource_view*   _srv;
};

class smaa
{
public:
    smaa(rendersys* rsys, int w, int h);
    ~smaa();
    void setup(rendersys* rsys);
    void go(shader_resource_view* src, shader_resource_view* velocity, render_target_view* dst, depth_stencil_view* dsv);
    void reproject();

protected:
    int                     _width, _height;
    smaa_render_target*     _edges_rt;
    smaa_render_target*     _blend_rt;
    render_texture2d*       _area_tex;
    shader_resource_view*   _area_srv;
    render_texture2d*       _search_tex;
    shader_resource_view*   _search_srv;
    int                     _frame_index;

protected:
    void initialize(rendersys* rsys);
    void initialize_fullscreen_triangle(rendersys* rsys);
    void destroy_miscs();
    void prepare_render_targets(rendersys* rsys, int w, int h);
    void load_area_tex(rendersys* rsys);
    void load_search_tex(rendersys* rsys);

protected:
    vertex_format*          _fullscreen_vf;
    vertex_shader*          _edge_detection_vs;
    vertex_shader*          _blending_weight_vs;
    vertex_shader*          _neighbor_blending_vs;
    pixel_shader*           _luma_edge_detection_ps;
    pixel_shader*           _color_edge_detection_ps;
    pixel_shader*           _blending_weight_ps;
    pixel_shader*           _neighbor_blending_ps;
    render_constant_buffer* _cb_configs;
    uint                    _cb_index;
    render_vertex_buffer*   _fullscreen_triangle;
};

__ariel_end__

#endif
