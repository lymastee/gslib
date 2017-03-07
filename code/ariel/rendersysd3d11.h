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

#ifndef rendersysd3d11_8555fd57_19e9_4747_97b5_8d3bfb0fb50f_h
#define rendersysd3d11_8555fd57_19e9_4747_97b5_8d3bfb0fb50f_h

#include <gslib/type.h>
#include <ariel/rendersys.h>

__ariel_begin__

class rendersys_d3d11:
    public rendersys
{
public:
    rendersys_d3d11();
    ~rendersys_d3d11();
    bool setup(uint hwnd, const configs& cfg) override;
    void destroy() override;
    void begin_create_shader(create_shader_context& context, const gchar* file, const gchar* entry, const gchar* sm, render_include* inc) override;
    void begin_create_shader_mem(create_shader_context& context, const char* src, int len, const gchar* name, const gchar* entry, const gchar* sm, render_include* inc) override;
    void end_create_shader(create_shader_context& context) override;
    vertex_shader* create_vertex_shader(create_shader_context& context) override;
    pixel_shader* create_pixel_shader(create_shader_context& context) override;
    compute_shader* create_compute_shader(create_shader_context& context) override;
    geometry_shader* create_geometry_shader(create_shader_context& context) override;
    hull_shader* create_hull_shader(create_shader_context& context) override;
    domain_shader* create_domain_shader(create_shader_context& context) override;
    vertex_format* create_vertex_format(create_shader_context& context, vertex_format_desc vfdesc[], uint n) override;
    vertex_buffer* create_vertex_buffer(uint stride, uint count, bool read, bool write, uint usage, const void* ptr) override;
    index_buffer* create_index_buffer(uint count, bool read, bool write, uint usage, const void* ptr) override;
    constant_buffer* create_constant_buffer(uint stride, bool read, bool write, const void* ptr) override;
    void update_buffer(void* buf, int size, const void* ptr) override;
    void set_vertex_format(vertex_format* vfmt) override;
    void set_vertex_buffer(vertex_buffer* vb, uint stride, uint offset) override;
    void set_index_buffer(index_buffer* ib, uint offset) override;
    void begin_render() override;
    void end_render() override;
    void set_render_option(render_option opt, uint val) override;
    void set_vertex_shader(vertex_shader* vs) override;
    void set_pixel_shader(pixel_shader* ps) override;
    void set_geometry_shader(geometry_shader* gs) override;
    void set_viewport(const viewport& vp) override;
    void set_constant_buffers(uint slot, constant_buffer* cb, shader_type st) override;
    void draw(uint count, uint start) override;
    void draw_indexed(uint count, uint start, int base) override;

protected:
    D3D_DRIVER_TYPE         _drvtype;
    D3D_FEATURE_LEVEL       _level;
    render_device*          _device;
    render_context*         _context;
    render_swap_chain*      _swapchain;
    render_target_view*     _rtview;
    bool                    _vsync;
    bool                    _fullscreen;

protected:
    void install_configs(const configs& cfg);
};

__ariel_end__

#endif
