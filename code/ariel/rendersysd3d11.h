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
    virtual void begin_create_shader(create_shader_context& context, const void* buf, int size) override;
    virtual void begin_create_shader_from_file(create_shader_context& context, const gchar* file, const gchar* entry, const gchar* sm, render_include* inc) override;
    virtual void begin_create_shader_from_memory(create_shader_context& context, const char* src, int len, const gchar* name, const gchar* entry, const gchar* sm, render_include* inc) override;
    virtual void end_create_shader(create_shader_context& context) override;
    virtual vertex_shader* create_vertex_shader(create_shader_context& context) override;
    virtual pixel_shader* create_pixel_shader(create_shader_context& context) override;
    virtual compute_shader* create_compute_shader(create_shader_context& context) override;
    virtual geometry_shader* create_geometry_shader(create_shader_context& context) override;
    virtual hull_shader* create_hull_shader(create_shader_context& context) override;
    virtual domain_shader* create_domain_shader(create_shader_context& context) override;
    virtual vertex_format* create_vertex_format(create_shader_context& context, vertex_format_desc vfdesc[], uint n) override;
    virtual vertex_buffer* create_vertex_buffer(uint stride, uint count, bool read, bool write, uint usage, const void* ptr) override;
    virtual index_buffer* create_index_buffer(uint count, bool read, bool write, uint usage, const void* ptr) override;
    virtual constant_buffer* create_constant_buffer(uint stride, bool read, bool write, const void* ptr) override;
    virtual shader_resource_view* create_shader_resource_view(render_resource* res) override;
    virtual sampler_state* create_sampler_state(sampler_state_filter filter) override;
    virtual texture2d* create_texture2d(const image& img, uint mips, uint usage, uint bindflags, uint cpuflags) override;
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

template<class res_class>
inline render_resource* convert_to_resource(res_class* p)
{ return static_cast<render_resource*>(p); }

__ariel_end__

#endif
