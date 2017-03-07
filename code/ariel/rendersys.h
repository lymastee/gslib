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

#ifndef rendersys_5fadb7a4_2c63_4b3a_8029_14043d2405e6_h
#define rendersys_5fadb7a4_2c63_4b3a_8029_14043d2405e6_h

#include <ariel/config.h>
#include <ariel/type.h>
#include <gslib/std.h>

__ariel_begin__

enum shader_type
{
    st_vertex_shader,
    st_pixel_shader,
    st_geometry_shader,
    st_hull_shader,
    st_domain_shader,
    st_tessellation_shader,
    st_compute_shader,
    /* more.. */
};

class __gs_novtable rendersys abstract
{
public:
    typedef unordered_map<string, string> configs;
    typedef render_vertex_buffer vertex_buffer;
    typedef render_index_buffer index_buffer;
    typedef render_constant_buffer constant_buffer;
    config_select_type(select_render_platform, create_shader_context);
    config_select_type(select_render_platform, vertex_format_desc);

public:
    virtual ~rendersys() {}
    virtual bool setup(uint hwnd, const configs& cfg) = 0;
    virtual void destroy() = 0;
    virtual void begin_create_shader(create_shader_context& context, const gchar* file, const gchar* entry, const gchar* sm, render_include* inc) = 0;
    virtual void begin_create_shader_mem(create_shader_context& context, const char* src, int len, const gchar* name, const gchar* entry, const gchar* sm, render_include* inc) = 0;
    virtual void end_create_shader(create_shader_context& context) = 0;
    virtual vertex_shader* create_vertex_shader(create_shader_context& context) = 0;
    virtual pixel_shader* create_pixel_shader(create_shader_context& context) = 0;
    virtual compute_shader* create_compute_shader(create_shader_context& context) = 0;
    virtual geometry_shader* create_geometry_shader(create_shader_context& context) = 0;
    virtual hull_shader* create_hull_shader(create_shader_context& context) = 0;
    virtual domain_shader* create_domain_shader(create_shader_context& context) = 0;
    virtual vertex_format* create_vertex_format(create_shader_context& context, vertex_format_desc desc[], uint n) = 0;
    virtual vertex_buffer* create_vertex_buffer(uint stride, uint count, bool read, bool write, uint usage, const void* ptr = 0) = 0;
    virtual index_buffer* create_index_buffer(uint count, bool read, bool write, uint usage, const void* ptr = 0) = 0;
    virtual constant_buffer* create_constant_buffer(uint stride, bool read, bool write, const void* ptr = 0) = 0;
    virtual void update_buffer(void* buf, int size, const void* ptr) = 0;
    virtual void set_vertex_format(vertex_format* vfmt) = 0;
    virtual void set_vertex_buffer(vertex_buffer* vb, uint stride, uint offset) = 0;
    virtual void set_index_buffer(index_buffer* ib, uint offset) = 0;
    virtual void begin_render() = 0;
    virtual void end_render() = 0;
    virtual void set_render_option(render_option opt, uint val) = 0;
    virtual void set_vertex_shader(vertex_shader* vs) = 0;
    virtual void set_pixel_shader(pixel_shader* ps) = 0;
    virtual void set_geometry_shader(geometry_shader* gs) = 0;
    virtual void set_viewport(const viewport& vp) = 0;
    virtual void set_constant_buffers(uint slot, constant_buffer* cb, shader_type st) = 0;
    virtual void draw(uint count, uint start) = 0;
    virtual void draw_indexed(uint count, uint start, int base = 0) = 0;

public:
    static bool is_vsync_enabled(const configs& cfg);
    static bool is_full_screen(const configs& cfg);
};

extern void release_vertex_buffer(render_vertex_buffer* buf);
extern void release_index_buffer(render_index_buffer* buf);
extern void release_constant_buffer(render_constant_buffer* buf);

__ariel_end__

#endif
