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
    virtual ~rendersys_d3d11();
    virtual bool setup(uint hwnd, const configs& cfg) override;
    virtual void destroy() override;
    virtual void setup_pipeline_state() override;
    virtual render_blob* compile_shader_from_file(const gchar* file, const gchar* entry, const gchar* sm, render_include* inc) override;
    virtual render_blob* compile_shader_from_memory(const char* src, int len, const gchar* name, const gchar* entry, const gchar* sm, render_include* inc) override;
    virtual vertex_shader* create_vertex_shader(const void* ptr, size_t len) override;
    virtual pixel_shader* create_pixel_shader(const void* ptr, size_t len) override;
    virtual compute_shader* create_compute_shader(const void* ptr, size_t len) override;
    virtual geometry_shader* create_geometry_shader(const void* ptr, size_t len) override;
    virtual hull_shader* create_hull_shader(const void* ptr, size_t len) override;
    virtual domain_shader* create_domain_shader(const void* ptr, size_t len) override;
    virtual vertex_format* create_vertex_format(const void* ptr, size_t len, vertex_format_desc vfdesc[], uint n) override;
    virtual vertex_buffer* create_vertex_buffer(uint stride, uint count, bool read, bool write, uint usage, const void* ptr) override;
    virtual index_buffer* create_index_buffer(uint count, bool read, bool write, uint usage, const void* ptr) override;
    virtual constant_buffer* create_constant_buffer(uint stride, bool read, bool write, const void* ptr) override;
    virtual shader_resource_view* create_shader_resource_view(render_resource* res) override;
    virtual depth_stencil_view* create_depth_stencil_view(render_resource* res) override;
    virtual unordered_access_view* create_unordered_access_view(render_resource* res) override;
    virtual sampler_state* create_sampler_state(sampler_state_filter filter) override;
    virtual texture2d* create_texture2d(const image& img, uint mips, uint usage, uint bindflags, uint cpuflags, uint miscflags) override;
    virtual texture2d* create_texture2d(int width, int height, uint format, uint mips, uint usage, uint bindflags, uint cpuflags, uint miscflags) override;
    virtual void load_with_mips(texture2d* tex, const image& img) override;
    virtual void update_buffer(void* buf, int size, const void* ptr) override;
    virtual void set_vertex_format(vertex_format* vfmt) override;
    virtual void set_vertex_buffer(vertex_buffer* vb, uint stride, uint offset) override;
    virtual void set_index_buffer(index_buffer* ib, uint offset) override;
    virtual void begin_render() override;
    virtual void end_render() override;
    virtual void set_render_option(render_option opt, uint val) override;
    virtual void set_vertex_shader(vertex_shader* vs) override;
    virtual void set_pixel_shader(pixel_shader* ps) override;
    virtual void set_geometry_shader(geometry_shader* gs) override;
    virtual void set_viewport(const viewport& vp) override;
    virtual void set_constant_buffer(uint slot, constant_buffer* cb, shader_type st) override;
    virtual void set_sampler_state(uint slot, sampler_state* sstate, shader_type st) override;
    virtual void set_shader_resource(uint slot, shader_resource_view* srv, shader_type st) override;
    virtual void draw(uint count, uint start) override;
    virtual void draw_indexed(uint count, uint start, int base) override;
    virtual void capture_screen(image& img, const rectf& rc, int buff_id) override;
    virtual void enable_alpha_blend(bool b) override;
    virtual void enable_depth(bool b) override;

protected:
    D3D_DRIVER_TYPE         _drvtype        = D3D_DRIVER_TYPE_NULL;
    D3D_FEATURE_LEVEL       _level          = D3D_FEATURE_LEVEL_11_0;
    render_device*          _device         = nullptr;
    render_context*         _context        = nullptr;
    render_swap_chain*      _swapchain      = nullptr;
    render_target_view*     _rtview         = nullptr;
    render_blend_state*     _blendstate     = nullptr;
    depth_stencil_view*     _dsview         = nullptr;
    render_raster_state*    _rasterstate    = nullptr;
    render_depth_state*     _depthstate     = nullptr;
    uint                    _msaa_x         = 0;
    bool                    _vsync          = false;
    bool                    _fullscreen     = false;
    bool                    _msaa           = false;

protected:
    void install_configs(const configs& cfg);

public:
    render_device* get_device() const { return _device; }
    render_context* get_immediate_context() const { return _context; }
};

template<class res_class>
inline render_resource* convert_to_resource(res_class* p)
{ return static_cast<render_resource*>(p); }

__ariel_end__

#endif
