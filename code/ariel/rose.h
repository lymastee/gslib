/*
 * Copyright (c) 2016 lymastee, All rights reserved.
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
#include <pink/raster.h>
#include <pink/utility.h>
#include <ariel/loopblinn.h>
#include <ariel/batch.h>

__ariel_begin__

using pink::raster;
using pink::graphic;
using pink::image;
using pink::painter_path;
using pink::color;
using pink::painter_brush;
using pink::painter_pen;

class rose;

/* this block was used for synchronize with the constant buffer */
struct rose_configs
{
    vec4                screen;
    float               mapscreen[3][4];
    /* more.. */
};

template<class _pack>
uint pack_cb_size()
{
    uint s = sizeof(_pack);
    uint m = s % 16;
    return !m ? s : s + (16 - m);
}

struct rose_bind_info
{
    vec4                color;      /* RGBA */
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

class rose_batch;
typedef list<rose_bind_info> rose_bind_list;
typedef vector<rose_batch*> rose_batch_list;
typedef vector<vertex_info_cr> vertex_stream_cr;
typedef vector<vertex_info_klm_cr> vertex_stream_klm_cr;
typedef vector<vertex_info_coef_cr> vertex_stream_cf_cr;
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

class rose_graphic:
    public graphic
{

};

extern void rose_paint_brush(loop_blinn_processor& lbp, rose_bind_list& bind_cache, const painter_brush& brush);
extern void rose_paint_solid_brush(loop_blinn_processor& lbp, rose_bind_list& bind_cache, const painter_brush& brush);
/* more to come. */

class rose:
    public raster
{
public:
    typedef render_constant_buffer constant_buffer;

public:
    rose();
    virtual ~rose();
    virtual void resize(int w, int h) override {}
    virtual void draw_path(const painter_path& path) override;
    virtual void on_draw_begin() override;
    virtual void on_draw_end() override;

public:
    void setup(rendersys* rsys);

protected:
    rendersys*          _rsys;
    rose_configs        _cfgs;
    batch_processor     _bp;
    rose_batch_list     _batches;
    rose_bind_list      _bindings;
    int                 _nextz;

protected:
    void setup_configs();
    void prepare_fill(const painter_path& path, const painter_brush& brush);
    void prepare_stroke(const painter_path& path, const painter_pen& pen);
    rose_batch* create_fill_batch_cr();
    rose_batch* create_fill_batch_klm_cr();
    rose_batch* create_stroke_batch_cr();
    void clear_batches();
    void prepare_batches();
    void draw_batches();

#if use_rendersys_d3d_11
protected:
    void initialize();
    void destroy_plugin();

protected:
    vertex_shader*      _vsf_cr;
    vertex_shader*      _vsf_klm_cr;
    pixel_shader*       _psf_cr;
    pixel_shader*       _psf_klm_cr;
    vertex_format*      _vf_cr;
    vertex_format*      _vf_klm_cr;
    vertex_shader*      _vss_coef_cr;
    pixel_shader*       _pss_coef_cr;
    vertex_format*      _vf_coef_cr;
    constant_buffer*    _cb_configs;
    uint                _cb_config_slot;
#endif
};

__ariel_end__

#endif
