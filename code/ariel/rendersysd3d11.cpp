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

#include <d3dcompiler.h>
#include <ariel/config.h>
#include <ariel/type.h>
#include <ariel/rendersysd3d11.h>
#include <gslib/entrywin32.h>

template<class Interface>
inline void SafeRelease(Interface*& pInterface)
{
    if(pInterface) {
        pInterface->Release();
        pInterface = 0;
    }
}

__ariel_begin__

rendersys_d3d11::rendersys_d3d11()
{
    _drvtype = D3D_DRIVER_TYPE_NULL;
    _level = D3D_FEATURE_LEVEL_11_0;
    _device = 0;
    _context = 0;
    _swapchain = 0;
    _rtview = 0;
    _vsync = false;
    _fullscreen = false;
}

rendersys_d3d11::~rendersys_d3d11()
{
    destroy();
}

bool rendersys_d3d11::setup(uint hwnd, const configs& cfg)
{
    /* install first */
    install_configs(cfg);
    /* setup */
    RECT rc;
    GetClientRect((HWND)hwnd, &rc);
    uint width = rc.right - rc.left;
    uint height = rc.bottom - rc.top;
    uint flags = 0;
#if defined (DEBUG) || defined (_DEBUG)
    //flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    D3D_DRIVER_TYPE drvtypes[] = 
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };
    uint cdrvtypes = _countof(drvtypes);
    D3D_FEATURE_LEVEL levels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };
    uint clevels = _countof(levels);
    /* get numerator & denominator */
    IDXGIFactory* factory = 0;
    IDXGIAdapter* adapter = 0;
    IDXGIOutput* adapter_output = 0;
    uint numerator, denominator, modes;
    if(FAILED(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory)) ||
        FAILED(factory->EnumAdapters(0, &adapter)) ||
        FAILED(adapter->EnumOutputs(0, &adapter_output)) ||
        FAILED(adapter_output->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &modes, 0))
        )
        return false;
    DXGI_MODE_DESC* display_modes = new DXGI_MODE_DESC[modes];
    assert(display_modes);
    if(FAILED(adapter_output->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &modes, display_modes))) {
        delete [] display_modes;
        return false;
    }
    int screen_width = GetSystemMetrics(SM_CXSCREEN);
    int screen_height = GetSystemMetrics(SM_CYSCREEN);
    for(uint i = 0; i < modes; i ++) {
        if((display_modes[i].Width == (uint)screen_width) &&
            (display_modes[i].Height = (uint)screen_height)
            ) {
            numerator = display_modes[i].RefreshRate.Numerator;
            denominator = display_modes[i].RefreshRate.Denominator;
            break;
        }
    }
    delete [] display_modes;
    /* create swap chain */
    DXGI_SWAP_CHAIN_DESC sd;
    memset(&sd, 0, sizeof(sd));
    sd.BufferCount          = 1;
    sd.BufferDesc.Width     = width;
    sd.BufferDesc.Height    = height;
    sd.BufferDesc.Format    = DXGI_FORMAT_R8G8B8A8_UNORM;
    if(_vsync) {
        sd.BufferDesc.RefreshRate.Numerator = numerator;
        sd.BufferDesc.RefreshRate.Denominator = denominator;
    }
    else {
        sd.BufferDesc.RefreshRate.Numerator = 0;
        sd.BufferDesc.RefreshRate.Denominator = 1;
    }
    sd.BufferUsage          = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow         = (HWND)hwnd;
    sd.SampleDesc.Count     = 1;
    sd.SampleDesc.Quality   = 0;
    sd.Windowed             = TRUE;
    if(_fullscreen)
        sd.Windowed = FALSE;
    for(uint i = 0; i < cdrvtypes; i ++) {
        _drvtype = drvtypes[i];
        if(SUCCEEDED(D3D11CreateDeviceAndSwapChain(0, _drvtype, 0, flags, levels, clevels, 
                D3D11_SDK_VERSION, &sd, &_swapchain, &_device, &_level, &_context))
            )
            break;
    }
    if(!_device || !_swapchain)
        return false;
    ID3D11Texture2D* buffer = 0;
    if(FAILED(_swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&buffer)))
        return false;
    bool fail = FAILED(_device->CreateRenderTargetView(buffer, 0, &_rtview));
    buffer->Release();
    if(fail)
        return false;
    _context->OMSetRenderTargets(1, &_rtview, 0);
    D3D11_VIEWPORT vp;
    vp.Width    = (FLOAT)width;
    vp.Height   = (FLOAT)height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    _context->RSSetViewports(1, &vp);
    /* enable alpha blending */
    ID3D11BlendState* blendstate = 0;
    D3D11_BLEND_DESC bd;
    memset(&bd, 0, sizeof(bd));
    bd.RenderTarget[0].BlendEnable = TRUE;
    bd.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    bd.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    bd.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    bd.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
    bd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    bd.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    fail = FAILED(_device->CreateBlendState(&bd, &blendstate));
    if(fail)
        return false;
    _context->OMSetBlendState(blendstate, 0, 0xffffffff);

//     D3D11_RASTERIZER_DESC rasdesc;
//     memset(&rasdesc, 0 ,sizeof(rasdesc));
//     rasdesc.FillMode = D3D11_FILL_SOLID;
//     rasdesc.CullMode = D3D11_CULL_NONE;
//     rasdesc.FrontCounterClockwise = FALSE;
//     rasdesc.DepthClipEnable = TRUE;
//     ID3D11RasterizerState* prs = 0;
//     _device->CreateRasterizerState(&rasdesc, &prs);
//     _context->RSSetState(prs);

    return true;
}

void rendersys_d3d11::destroy()
{
    if(_context)
        _context->ClearState();
    SafeRelease(_rtview);
    SafeRelease(_swapchain);
    SafeRelease(_context);
    SafeRelease(_device);
}

void rendersys_d3d11::begin_create_shader(create_shader_context& context, const gchar* file, const gchar* entry, const gchar* sm, render_include* inc)
{
    HRESULT hr = S_OK;
    DWORD flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined (DEBUG) || defined (_DEBUG)
    flags |= D3DCOMPILE_DEBUG;
#endif
    render_blob* err_blob = 0;
    _string<char> sbentry, sbsmodel;
    sbentry.from(entry);
    sbsmodel.from(sm);
    _string<wchar> mbfile;
    mbfile.from(file);
    hr = D3DCompileFromFile(mbfile.c_str(), 0, inc, sbentry.c_str(), sbsmodel.c_str(), flags, 0, &context, &err_blob);
    if(FAILED(hr)) {
        if(err_blob)
            OutputDebugStringA((char*)err_blob->GetBufferPointer());
        SafeRelease(err_blob);
        return;
    }
    SafeRelease(err_blob);
}

void rendersys_d3d11::begin_create_shader_mem(create_shader_context& context, const char* src, int len, const gchar* name, const gchar* entry, const gchar* sm, render_include* inc)
{
    HRESULT hr = S_OK;
    DWORD flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined (DEBUG) || defined (_DEBUG)
    flags |= D3DCOMPILE_DEBUG;
#endif
    render_blob* err_blob = 0;
    _string<char> sbname, sbentry, sbsmodel;
    sbentry.from(entry);
    sbsmodel.from(sm);
    sbname.from(name);
    hr = D3DCompile(src, len, sbname.c_str(), 0, inc, sbentry.c_str(), sbsmodel.c_str(), flags, 0, &context, &err_blob);
    if(FAILED(hr)) {
        if(err_blob)
            OutputDebugStringA((char*)err_blob->GetBufferPointer());
        SafeRelease(err_blob);
        return;
    }
    SafeRelease(err_blob);
}

void rendersys_d3d11::end_create_shader(create_shader_context& context)
{
    SafeRelease(context);
}

vertex_shader* rendersys_d3d11::create_vertex_shader(create_shader_context& context)
{
    if(!context)
        return 0;
    assert(_device);
    vertex_shader* shader = 0;
    return FAILED(_device->CreateVertexShader(context->GetBufferPointer(), context->GetBufferSize(), 0, &shader)) ? 0 : shader;
}

pixel_shader* rendersys_d3d11::create_pixel_shader(create_shader_context& context)
{
    if(!context)
        return 0;
    pixel_shader* shader = 0;
    assert(_device);
    return FAILED(_device->CreatePixelShader(context->GetBufferPointer(), context->GetBufferSize(), 0, &shader)) ? 0 : shader;
}

compute_shader* rendersys_d3d11::create_compute_shader(create_shader_context& context)
{
    if(!context)
        return 0;
    compute_shader* shader = 0;
    assert(_device);
    return FAILED(_device->CreateComputeShader(context->GetBufferPointer(), context->GetBufferSize(), 0, &shader)) ? 0 : shader;
}

geometry_shader* rendersys_d3d11::create_geometry_shader(create_shader_context& context)
{
    if(!context)
        return 0;
    geometry_shader* shader = 0;
    assert(_device);
    return FAILED(_device->CreateGeometryShader(context->GetBufferPointer(), context->GetBufferSize(), 0, &shader)) ? 0 : shader;
}

hull_shader* rendersys_d3d11::create_hull_shader(create_shader_context& context)
{
    if(!context)
        return 0;
    hull_shader* shader = 0;
    assert(_device);
    return FAILED(_device->CreateHullShader(context->GetBufferPointer(), context->GetBufferSize(), 0, &shader)) ? 0 : shader;
}

domain_shader* rendersys_d3d11::create_domain_shader(create_shader_context& context)
{
    if(!context)
        return 0;
    domain_shader* shader = 0;
    assert(_device);
    return FAILED(_device->CreateDomainShader(context->GetBufferPointer(), context->GetBufferSize(), 0, &shader)) ? 0 : shader;
}

vertex_format* rendersys_d3d11::create_vertex_format(create_shader_context& context, vertex_format_desc desc[], uint n)
{
    vertex_format* format = 0;
    assert(_device);
    return FAILED(_device->CreateInputLayout(desc, n, context->GetBufferPointer(), context->GetBufferSize(), &format)) ? 0 : format;
}

render_vertex_buffer* rendersys_d3d11::create_vertex_buffer(uint stride, uint count, bool read, bool write, uint usage, const void* ptr)
{
    D3D11_BUFFER_DESC desc;
    memset(&desc, 0, sizeof(desc));
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.ByteWidth = stride * count;
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    desc.CPUAccessFlags = 0;
    if(read != false)
        desc.CPUAccessFlags |= D3D11_CPU_ACCESS_READ;
    if(write != false)
        desc.CPUAccessFlags |= D3D11_CPU_ACCESS_WRITE;
    D3D11_SUBRESOURCE_DATA* psd = 0, sd;
    if(ptr) {
        memset(&sd, 0, sizeof(sd));
        sd.pSysMem = ptr;
        psd = &sd;
    }
    vertex_buffer* vb = 0;
    return FAILED(_device->CreateBuffer(&desc, psd, &vb)) ? 0 : vb;
}

render_index_buffer* rendersys_d3d11::create_index_buffer(uint count, bool read, bool write, uint usage, const void* ptr)
{
    D3D11_BUFFER_DESC desc;
    memset(&desc, 0, sizeof(desc));
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.ByteWidth = count << 2;
    desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    desc.CPUAccessFlags = 0;
    if(read != false)
        desc.CPUAccessFlags |= D3D11_CPU_ACCESS_READ;
    if(write != false)
        desc.CPUAccessFlags |= D3D11_CPU_ACCESS_WRITE;
    D3D11_SUBRESOURCE_DATA* psd = 0, sd;
    if(ptr) {
        memset(&sd, 0, sizeof(sd));
        sd.pSysMem = ptr;
        psd = &sd;
    }
    index_buffer* ib;
    return FAILED(_device->CreateBuffer(&desc, psd, &ib)) ? 0 : ib;
}

render_constant_buffer* rendersys_d3d11::create_constant_buffer(uint stride, bool read, bool write, const void* ptr)
{
    D3D11_BUFFER_DESC desc;
    memset(&desc, 0, sizeof(desc));
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.MiscFlags = 0;
    desc.StructureByteStride = 0;
    desc.ByteWidth = stride;
    desc.CPUAccessFlags = 0;
    if(read != false)
        desc.CPUAccessFlags |= D3D11_CPU_ACCESS_READ;
    if(write != false)
        desc.CPUAccessFlags |= D3D11_CPU_ACCESS_WRITE;
    D3D11_SUBRESOURCE_DATA* psd = 0, sd;
    if(ptr) {
        memset(&sd, 0, sizeof(sd));
        sd.pSysMem = ptr;
        psd = &sd;
    }
    constant_buffer* cb;
    return FAILED(_device->CreateBuffer(&desc, psd, &cb)) ? 0 : cb;
}

void rendersys_d3d11::update_buffer(void* buf, int size, const void* ptr)
{
    assert(buf && ptr && size > 0);
    D3D11_MAPPED_SUBRESOURCE mapres;
    _context->Map((ID3D11Buffer*)buf, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapres);
    memcpy_s(mapres.pData, size, ptr, size);
    _context->Unmap((ID3D11Buffer*)buf, 0);
}

void rendersys_d3d11::set_vertex_format(vertex_format* vfmt)
{
    assert(_context);
    _context->IASetInputLayout(vfmt);
}

void rendersys_d3d11::set_vertex_buffer(vertex_buffer* vb, uint stride, uint offset)
{
    assert(_context);
    _context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
}

void rendersys_d3d11::set_index_buffer(index_buffer* ib, uint offset)
{
    assert(_context);
    _context->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, offset);
}

void rendersys_d3d11::begin_render()
{
    assert(_context);
    static float ccr[] = { 0.f, 0.f, 0.f, 1.f };
    _context->ClearRenderTargetView(_rtview, ccr);
}

void rendersys_d3d11::end_render()
{
    assert(_swapchain);
    _swapchain->Present(_vsync ? 1 : 0, 0);
}

void rendersys_d3d11::set_render_option(render_option opt, uint val)
{
    assert(_context);
    switch(opt)
    {
    case opt_primitive_topology:
        _context->IASetPrimitiveTopology((D3D11_PRIMITIVE_TOPOLOGY)val);
    }
}

void rendersys_d3d11::set_vertex_shader(vertex_shader* vs)
{
    assert(_context);
    _context->VSSetShader(vs, 0, 0);
}

void rendersys_d3d11::set_pixel_shader(pixel_shader* ps)
{
    assert(_context);
    _context->PSSetShader(ps, 0, 0);
}

void rendersys_d3d11::set_geometry_shader(geometry_shader* gs)
{
    assert(_context);
    _context->GSSetShader(gs, 0, 0);
}

void rendersys_d3d11::set_viewport(const viewport& vp)
{
    assert(_context);
    D3D11_VIEWPORT dvp;
    dvp.Width = vp.width;
    dvp.Height = vp.height;
    dvp.MaxDepth = vp.max_depth;
    dvp.MinDepth = vp.min_depth;
    dvp.TopLeftX = vp.left;
    dvp.TopLeftY = vp.top;
    _context->RSSetViewports(1, &dvp);
}

void rendersys_d3d11::set_constant_buffers(uint slot, constant_buffer* cb, shader_type st)
{
    assert(cb);
    switch(st)
    {
    case st_vertex_shader:
        _context->VSSetConstantBuffers(slot, 1, &cb);
        break;
    case st_pixel_shader:
        _context->PSSetConstantBuffers(slot, 1, &cb);
        break;
    default:
        assert(!"unknown shader type.");
    }
}

void rendersys_d3d11::draw(uint count, uint start)
{
    assert(_context);
    _context->Draw(count, start);
}

void rendersys_d3d11::draw_indexed(uint count, uint start, int base)
{
    assert(_context);
    _context->DrawIndexed(count, start, base);
}

void rendersys_d3d11::install_configs(const configs& cfg)
{
    _vsync = is_vsync_enabled(cfg);
    _fullscreen = is_full_screen(cfg);
}

void release_vertex_buffer(render_vertex_buffer* buf) { if(buf) buf->Release(); }
void release_index_buffer(render_index_buffer* buf) { if(buf) buf->Release(); }
void release_constant_buffer(render_constant_buffer* buf) { if(buf) buf->Release(); }

__ariel_end__
