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

#ifndef config_27ec2780_3d0a_41f9_8a8b_0099ced77cf1_h
#define config_27ec2780_3d0a_41f9_8a8b_0099ced77cf1_h

#include <gslib/config.h>

#define __ariel_begin__     namespace gs { namespace ariel {
#define __ariel_end__       } };

#if defined(ARIEL_LIB)
#define ariel_export
#elif defined(ARIEL_DLL)
#define ariel_export __declspec(dllexport)
#else
#define ariel_export __declspec(dllimport)
#endif

enum
{
    render_platform_gl_20,
    render_platform_gl_30,
    render_platform_d3d_9,
    render_platform_d3d_11,
};

/* select render system here. */
#define select_render_platform  render_platform_d3d_11

#define use_rendersys_d3d_9     (select_render_platform == render_platform_d3d_9)
#define use_rendersys_d3d_11    (select_render_platform == render_platform_d3d_11)
#define use_rendersys_gl_20     (select_render_platform == render_platform_gl_20)
#define use_rendersys_gl_30     (select_render_platform == render_platform_gl_30)

#if use_rendersys_d3d_11
#include <d3d11.h>
#include <dxgi.h>
#endif

__ariel_begin__

define_select_type(render_device);
define_select_type(render_context);
define_select_type(render_resource);
define_select_type(render_swap_chain);
define_select_type(render_target_view);
define_select_type(shader_resource_view);
define_select_type(depth_stencil_view);
define_select_type(unordered_access_view);
define_select_type(render_blob);
define_select_type(vertex_shader);
define_select_type(pixel_shader);
define_select_type(geometry_shader);
define_select_type(compute_shader);
define_select_type(hull_shader);
define_select_type(domain_shader);
define_select_type(render_include);
define_select_type(vertex_format);
define_select_type(vertex_format_desc);
define_select_type(render_vertex_buffer);
define_select_type(render_index_buffer);
define_select_type(render_constant_buffer);
define_select_type(render_texture1d);
define_select_type(render_texture2d);
define_select_type(render_texture3d);
define_select_type(render_sampler_state);
define_select_type(render_blend_state);
define_select_type(render_raster_state);
define_select_type(render_depth_state);

install_select_type(render_platform_d3d_11, render_device, ID3D11Device);
install_select_type(render_platform_d3d_11, render_context, ID3D11DeviceContext);
install_select_type(render_platform_d3d_11, render_resource, ID3D11Resource);
install_select_type(render_platform_d3d_11, render_swap_chain, IDXGISwapChain);
install_select_type(render_platform_d3d_11, render_target_view, ID3D11RenderTargetView);
install_select_type(render_platform_d3d_11, shader_resource_view, ID3D11ShaderResourceView);
install_select_type(render_platform_d3d_11, depth_stencil_view, ID3D11DepthStencilView);
install_select_type(render_platform_d3d_11, unordered_access_view, ID3D11UnorderedAccessView);
install_select_type(render_platform_d3d_11, render_blob, ID3DBlob);
install_select_type(render_platform_d3d_11, vertex_shader, ID3D11VertexShader);
install_select_type(render_platform_d3d_11, pixel_shader, ID3D11PixelShader);
install_select_type(render_platform_d3d_11, geometry_shader, ID3D11GeometryShader);
install_select_type(render_platform_d3d_11, compute_shader, ID3D11ComputeShader);
install_select_type(render_platform_d3d_11, hull_shader, ID3D11HullShader);
install_select_type(render_platform_d3d_11, domain_shader, ID3D11DomainShader);

install_select_type(render_platform_d3d_11, render_include, ID3DInclude);
install_select_type(render_platform_d3d_11, vertex_format, ID3D11InputLayout);
install_select_type(render_platform_d3d_11, vertex_format_desc, D3D11_INPUT_ELEMENT_DESC);
install_select_type(render_platform_d3d_11, render_vertex_buffer, ID3D11Buffer);
install_select_type(render_platform_d3d_11, render_index_buffer, ID3D11Buffer);
install_select_type(render_platform_d3d_11, render_constant_buffer, ID3D11Buffer);

install_select_type(render_platform_d3d_11, render_texture1d, ID3D11Texture1D);
install_select_type(render_platform_d3d_11, render_texture2d, ID3D11Texture2D);
install_select_type(render_platform_d3d_11, render_texture3d, ID3D11Texture3D);
install_select_type(render_platform_d3d_11, render_sampler_state, ID3D11SamplerState);
install_select_type(render_platform_d3d_11, render_blend_state, ID3D11BlendState);
install_select_type(render_platform_d3d_11, render_raster_state, ID3D11RasterizerState);
install_select_type(render_platform_d3d_11, render_depth_state, ID3D11DepthStencilState);

config_select_type(select_render_platform, render_device);
config_select_type(select_render_platform, render_context);
config_select_type(select_render_platform, render_resource);
config_select_type(select_render_platform, render_swap_chain);
config_select_type(select_render_platform, render_target_view);
config_select_type(select_render_platform, shader_resource_view);
config_select_type(select_render_platform, depth_stencil_view);
config_select_type(select_render_platform, unordered_access_view);
config_select_type(select_render_platform, render_blob);
config_select_type(select_render_platform, vertex_shader);
config_select_type(select_render_platform, pixel_shader);
config_select_type(select_render_platform, geometry_shader);
config_select_type(select_render_platform, compute_shader);
config_select_type(select_render_platform, hull_shader);
config_select_type(select_render_platform, domain_shader);
config_select_type(select_render_platform, render_include);
config_select_type(select_render_platform, vertex_format);
config_select_type(select_render_platform, render_vertex_buffer);
config_select_type(select_render_platform, render_index_buffer);
config_select_type(select_render_platform, render_constant_buffer);
config_select_type(select_render_platform, render_texture1d);
config_select_type(select_render_platform, render_texture2d);
config_select_type(select_render_platform, render_texture3d);
config_select_type(select_render_platform, render_sampler_state);
config_select_type(select_render_platform, render_blend_state);
config_select_type(select_render_platform, render_raster_state);
config_select_type(select_render_platform, render_depth_state);

enum render_option
{
    opt_primitive_topology,
    /* more to come. */
};

__ariel_end__

#endif
