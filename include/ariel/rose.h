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

#ifndef rose_fbed1fbf_2ba3_46bc_97da_b1df11752358_h
#define rose_fbed1fbf_2ba3_46bc_97da_b1df11752358_h

#include <ariel/config.h>
#include <ariel/rendersys.h>
#include <ariel/raster.h>
#include <ariel/utility.h>
#include <ariel/batch.h>
#include <ariel/texbatch.h>

__ariel_begin__

class rose;

/* this block was used for synchronize with the constant buffer */
struct rose_configs
{
    vec4                screen;
    float               mapscreen[3][4];
    /* more.. */
};

struct rose_bind_info_cr
{
    vec4                color;      /* RGBA */
};

struct rose_bind_info_tex
{
    texture2d*          img;
    vec2                tex;
};

struct vertex_info_cr
{
    vec2                pos;
    vec4                cr;
};

struct vertex_info_klm_cr
{
    vec2                pos;
    vec3                klm;
    vec4                cr;
};

struct vertex_info_coef_cr
{
    vec2                pos;
    vec4                coef;       /* float3 coef & float tune */
    vec4                cr;
};

struct vertex_info_klm_tex
{
    vec2                pos;
    vec3                klm;
    vec2                tex;
};

struct vertex_info_coef_tex
{
    vec2                pos;
    vec4                coef;
    vec2                tex;
};

class rose_batch;
typedef list<rose_bind_info_cr> rose_bind_list_cr;
typedef list<rose_bind_info_tex> rose_bind_list_tex;
typedef vector<rose_batch*> rose_batch_list;
typedef vector<vertex_info_cr> vertex_stream_cr;
typedef vector<vertex_info_klm_cr> vertex_stream_klm_cr;
typedef vector<vertex_info_coef_cr> vertex_stream_cf_cr;
typedef vector<vertex_info_klm_tex> vertex_stream_klm_tex;
typedef vector<vertex_info_coef_tex> vertex_stream_cf_tex;
typedef vector<int> index_stream;
typedef bat_type rose_batch_tag;

class __gs_novtable rose_batch abstract
{
public:
    typedef rendersys::vertex_buffer vertex_buffer;
    typedef rendersys::index_buffer index_buffer;

public:
    rose_batch();
    virtual ~rose_batch();
    virtual rose_batch_tag get_tag() const = 0;
    virtual void create(bat_batch* bat) = 0;
    virtual int buffering(rendersys* rsys) = 0;
    virtual void draw(rendersys* rsys) = 0;
    virtual void tracing() const = 0;

protected:
    vertex_shader*      _vertex_shader;
    pixel_shader*       _pixel_shader;
    vertex_format*      _vertex_format;
    vertex_buffer*      _vertex_buffer;

public:
    void set_vertex_shader(vertex_shader* p) { _vertex_shader = p; }
    void set_pixel_shader(pixel_shader* p) { _pixel_shader = p; }
    void set_vertex_format(vertex_format* p) { _vertex_format = p; }
    void setup_vs_and_ps(rendersys* rsys);
    void setup_vf_and_topology(rendersys* rsys, uint topo);

protected:
    template<class stream_type>
    int template_buffering(stream_type& stm, rendersys* rsys);
};

class rose_fill_batch_cr:
    public rose_batch
{
public:
    rose_batch_tag get_tag() const override { return bf_cr; }
    void create(bat_batch* bat) override;
    int buffering(rendersys* rsys) override;
    void draw(rendersys* rsys) override;
    void tracing() const override;

protected:
    vertex_stream_cr    _vertices;

private:
    void create_from_fill(bat_fill_batch* bat);
    void create_from_stroke(bat_stroke_batch* bat);
};

class rose_fill_batch_klm_cr:
    public rose_batch
{
public:
    rose_batch_tag get_tag() const override { return bf_klm_cr; }
    void create(bat_batch* bat) override;
    int buffering(rendersys* rsys) override;
    void draw(rendersys* rsys) override;
    void tracing() const override;

protected:
    vertex_stream_klm_cr _vertices;
};

class rose_fill_batch_klm_tex:
    public rose_batch
{
    friend class rose_stroke_batch_assoc_with_klm_tex;

public:
    rose_fill_batch_klm_tex(render_sampler_state* ss);
    ~rose_fill_batch_klm_tex() { destroy(); }
    rose_batch_tag get_tag() const override { return bf_klm_tex; }
    void create(bat_batch* bat) override;
    int buffering(rendersys* rsys) override;
    void draw(rendersys* rsys) override;
    void tracing() const override;
    void destroy();

protected:
    vertex_stream_klm_tex _vertices;
    tex_batcher         _texbatch;
    render_sampler_state* _sstate;
    shader_resource_view* _srv;
    render_texture2d*   _tex;

private:
    void create_from_fill(bat_fill_batch* bat);
    void create_from_stroke(bat_stroke_batch* bat);
};

class rose_stroke_batch_coef_cr:
    public rose_batch
{
public:
    rose_batch_tag get_tag() const override { return bs_coef_cr; }
    void create(bat_batch* bat) override;
    int buffering(rendersys* rsys) override;
    void draw(rendersys* rsys) override;
    void tracing() const override;

protected:
    vertex_stream_cf_cr _vertices;
};

class rose_stroke_batch_coef_tex:
    public rose_batch
{
public:
    rose_stroke_batch_coef_tex(render_sampler_state* ss);
    ~rose_stroke_batch_coef_tex() { destroy(); }
    rose_batch_tag get_tag() const override { return bs_coef_tex; }
    void create(bat_batch* bat) override;
    int buffering(rendersys* rsys) override;
    void draw(rendersys* rsys) override;
    void tracing() const override;
    void destroy();

protected:
    vertex_stream_cf_tex _vertices;
    tex_batcher         _texbatch;
    render_sampler_state* _sstate;
    shader_resource_view* _srv;
    render_texture2d*   _tex;

protected:
    void create_vertices(bat_batch* bat);
};

class rose_stroke_batch_assoc_with_klm_tex:
    public rose_stroke_batch_coef_tex
{
public:
    rose_stroke_batch_assoc_with_klm_tex(rose_fill_batch_klm_tex* assoc);
    ~rose_stroke_batch_assoc_with_klm_tex();
    void create(bat_batch* bat) override;
    int buffering(rendersys* rsys) override;

protected:
    rose_fill_batch_klm_tex* _assoc;
};

class rose_bindings
{
public:
    rose_bind_list_cr& get_cr_bindings() { return _cr_bindings; }
    rose_bind_list_tex& get_tex_bindings() { return _tex_bindings; }
    void clear_binding_cache();

protected:
    rose_bind_list_cr   _cr_bindings;
    rose_bind_list_tex  _tex_bindings;
};

class graphics_obj_entity:
    public loop_blinn_processor
{
public:
    graphics_obj_entity(float w, float h) : loop_blinn_processor(w, h) {}
    void proceed_fill(const painter_path& path) { __super::proceed(path); }
    void proceed_stroke(const painter_path& path);

protected:
    int create_from_path(const painter_path& path, int start);

    struct path_seg
    {
        lb_line*        first;
        lb_line*        last;
        path_seg() { first = last = nullptr; }
    };
    void add_line_seg(path_seg& seg, const painter_path::line_to_node* node);
    void add_quad_seg(path_seg& seg, const painter_node* node1, const painter_path::quad_to_node* node2);
    void add_cubic_seg(path_seg& seg, const painter_node* node1, const painter_path::cubic_to_node* node2);
};

class graphics_obj:
    public std::shared_ptr<graphics_obj_entity>
{
public:
    graphics_obj(float w, float h);
};

extern void rose_paint_non_picture_brush(graphics_obj& gfx, rose_bindings& bindings, const painter_brush& brush);
extern void rose_paint_solid_brush(graphics_obj& gfx, rose_bind_list_cr& bind_cache, const painter_brush& brush);
extern void rose_paint_picture_brush(graphics_obj& gfx, const rectf& bound, rose_bind_list_tex& bind_cache, const painter_brush& brush);
extern void rose_paint_pen(graphics_obj& gfx, const painter_path& path, rose_bindings& bindings, const painter_pen& pen);
extern void rose_paint_solid_pen(graphics_obj& gfx, rose_bind_list_cr& bind_cache, const painter_pen& pen);
extern void rose_paint_picture_pen(graphics_obj& gfx, const rectf& bound, rose_bind_list_tex& bind_cache, const painter_pen& pen);
/* more to come. */

class rose:
    public raster
{
public:
    typedef render_constant_buffer constant_buffer;
    typedef render_sampler_state sampler_state;
    typedef list<graphics_obj> graphics_obj_cache;

public:
    rose();
    virtual ~rose();
    virtual void resize(int w, int h) override {}
    virtual void draw_path(const painter_path& path) override;
    virtual void on_draw_begin() override;
    virtual void on_draw_end() override;

public:
    void setup(rendersys* rsys);
    void fill_non_picture_graphics_obj(graphics_obj& gfx, uint brush_tag);
    bat_batch* fill_picture_graphics_obj(graphics_obj& gfx);
    void stroke_graphics_obj(graphics_obj& gfx, uint pen_tag);

protected:
    template<class _addline>
    void meta_stroke_graphics_obj(graphics_obj& gfx, _addline fn_add);

protected:
    rendersys*          _rsys;
    rose_configs        _cfgs;
    batch_processor     _bp;
    rose_batch_list     _batches;
    rose_bindings       _bindings;
    graphics_obj_cache  _gocache;
    float               _nextz;

protected:
    void setup_configs();
    void prepare_fill(const painter_path& path, const painter_brush& brush);
    void prepare_picture_fill(const painter_path& path, const painter_brush& brush);
    void prepare_stroke(const painter_path& path, const painter_pen& pen);
    rose_batch* create_fill_batch_cr();
    rose_batch* create_fill_batch_klm_cr();
    rose_batch* create_fill_batch_klm_tex();
    rose_batch* create_stroke_batch_cr();
    rose_batch* create_stroke_batch_tex();
    rose_batch* create_stroke_batch_assoc(rose_fill_batch_klm_tex* assoc);
    void clear_batches();
    void prepare_batches();
    void draw_batches();

#if use_rendersys_d3d_11
protected:
    void initialize();
    void destroy_miscs();
    sampler_state* acquire_default_sampler_state();

protected:
    vertex_shader*      _vsf_cr;
    vertex_shader*      _vsf_klm_cr;
    vertex_shader*      _vsf_klm_tex;
    pixel_shader*       _psf_cr;
    pixel_shader*       _psf_klm_cr;
    pixel_shader*       _psf_klm_tex;
    vertex_format*      _vf_cr;
    vertex_format*      _vf_klm_cr;
    vertex_format*      _vf_klm_tex;
    vertex_shader*      _vss_coef_cr;
    vertex_shader*      _vss_coef_tex;
    pixel_shader*       _pss_coef_cr;
    pixel_shader*       _pss_coef_tex;
    vertex_format*      _vf_coef_cr;
    vertex_format*      _vf_coef_tex;
    sampler_state*      _sampler_state;
    constant_buffer*    _cb_configs;
    uint                _cb_config_slot;
#endif
};

__ariel_end__

#endif
