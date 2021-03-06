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

#include <d3dcommon.h>
#include <d3dcompiler.h>
#include <ariel/config.h>
#include <ariel/type.h>
#include <ariel/rendersysd3d11.h>
#include <ariel/textureop.h>

template<class Interface>
inline void SafeRelease(Interface*& pInterface)
{
    if(pInterface) {
        pInterface->Release();
        pInterface = nullptr;
    }
}

static UINT convert_msaa_x(UINT x)
{
    if(x >= 16)
        return 16;
    DWORD i = 0;
    _BitScanReverse(&i, x);
    switch(i)       /* pow faster or slower? */
    {
    case 3:
        return 8;
    case 2:
        return 4;
    case 1:
        return 2;
    }
    return 1;
}

static UINT next_msaa_x(UINT x)
{
    return x >> 1;
}

__ariel_begin__

static void setup_device_info(render_device_info& info, IDXGIAdapter* adapter)
{
    assert(adapter);
    DXGI_ADAPTER_DESC desc;
    if(FAILED(adapter->GetDesc(&desc)))
        return;
    info.vendor_id = desc.VendorId;
}

rendersys_d3d11::rendersys_d3d11()
{
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
    uint flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;      /* for d2d interop */
#if defined (DEBUG) || defined (_DEBUG)
    flags |= D3D11_CREATE_DEVICE_DEBUG;
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
    IDXGIFactory* factory = nullptr;
    IDXGIAdapter* adapter = nullptr;
    IDXGIOutput* adapter_output = nullptr;
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
    /* setup device info */
    setup_device_info(_device_info, adapter);
    /* create device */
    for(uint i = 0; i < cdrvtypes; i ++) {
        _drvtype = drvtypes[i];
        if(SUCCEEDED(D3D11CreateDevice(nullptr, _drvtype, 0, flags, levels, clevels, D3D11_SDK_VERSION, &_device, &_level, &_context)))
            break;
    }
    if(!_device || !_context)
        return false;
    /* query MSAA support */
    uint sampler_count = _msaa_x, sampler_quality = 0;
    if(_msaa) {
        for(; sampler_count >= 1; sampler_count = next_msaa_x(sampler_count)) {
            _device->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, sampler_count, &sampler_quality);
            if(sampler_quality != 0)
                break;
        }
        if(sampler_count > 1)
            -- sampler_quality;
        else {
            sampler_count = 1;
            sampler_quality = 0;
            _msaa_x = 1;
            _msaa = false;      /* unsupported */
        }
    }
    else {
        _msaa_x = 1;
        sampler_count = 1;
        sampler_quality = 0;
    }
    /* create swap chain */
    DXGI_SWAP_CHAIN_DESC sd;
    memset(&sd, 0, sizeof(sd));
    sd.BufferCount          = 2;
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
    sd.SampleDesc.Count     = sampler_count;
    sd.SampleDesc.Quality   = sampler_quality;
    sd.Windowed             = TRUE;
    sd.SwapEffect           = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    if(_msaa)
        sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    if(_fullscreen)
        sd.Windowed = FALSE;
    assert(factory);
    if(FAILED(factory->CreateSwapChain(_device, &sd, &_swapchain)) || !_swapchain)
        return false;
    com_ptr<ID3D11Texture2D> buffer;
    if(FAILED(_swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&buffer)))
        return false;
    bool fail = FAILED(_device->CreateRenderTargetView(buffer.get(), 0, &_rtview));
    if(fail)
        return false;
    /* create blend state */
    com_ptr<ID3D11BlendState> blendstate;
    D3D11_BLEND_DESC bd;
    memset(&bd, 0, sizeof(bd));
    bd.RenderTarget[0].BlendEnable = TRUE;
    bd.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    bd.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    bd.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    bd.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
    bd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_DEST_ALPHA;
    bd.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_MAX;
    bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    fail = FAILED(_device->CreateBlendState(&bd, &blendstate));
    if(fail)
        return false;
    _blendstate = blendstate.detach();
    /* create depth stencil buffer */
    D3D11_TEXTURE2D_DESC dsbd;
    memset(&dsbd, 0, sizeof(dsbd));
    dsbd.Width              = width;
    dsbd.Height             = height;
    dsbd.MipLevels          = 1;
    dsbd.ArraySize          = 1;
    dsbd.Format             = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsbd.SampleDesc.Count   = sampler_count;
    dsbd.SampleDesc.Quality = sampler_quality;
    dsbd.Usage              = D3D11_USAGE_DEFAULT;
    dsbd.BindFlags          = D3D11_BIND_DEPTH_STENCIL;
    dsbd.CPUAccessFlags     = 0;
    dsbd.MiscFlags          = 0;
    com_ptr<ID3D11Texture2D> dsbuffer;
    if(FAILED(_device->CreateTexture2D(&dsbd, nullptr, &dsbuffer)))
        return false;
    /* create depth stencil view */
    _dsview = create_depth_stencil_view(dsbuffer.get());
    if(!_dsview)
        return false;
    /* create depth stencil state */
    com_ptr<ID3D11DepthStencilState> dsoffstate;
    D3D11_DEPTH_STENCIL_DESC dsoffd;
    memset(&dsoffd, 0, sizeof(dsoffd));
    dsoffd.DepthEnable = FALSE;
    dsoffd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    dsoffd.DepthFunc = D3D11_COMPARISON_NEVER;
    dsoffd.StencilEnable = FALSE;
    if(FAILED(_device->CreateDepthStencilState(&dsoffd, &dsoffstate)))
        return false;
    _depthstate = dsoffstate.detach();
    /* enable MSAA */
    if(_msaa) {
        D3D11_RASTERIZER_DESC rd;
        memset(&rd, 0, sizeof(rd));
        rd.AntialiasedLineEnable    = true;
        rd.CullMode                 = D3D11_CULL_BACK;
        rd.DepthBias                = 0;
        rd.DepthBiasClamp           = 0.f;
        rd.DepthClipEnable          = true;
        rd.FillMode                 = D3D11_FILL_SOLID;
        rd.FrontCounterClockwise    = false;
        rd.MultisampleEnable        = true;
        rd.ScissorEnable            = false;
        rd.SlopeScaledDepthBias     = 0.0f;
        com_ptr<ID3D11RasterizerState>  rasterstate;
        fail = FAILED(_device->CreateRasterizerState(&rd, &rasterstate));
        if(fail)
            return false;
        _rasterstate = rasterstate.detach();
    }
    register_dev_index_service(_device, this);
    return true;
}

void rendersys_d3d11::destroy()
{
    unregister_dev_index_service(_device);
    if(_context)
        _context->ClearState();
    SafeRelease(_blendstate);
    SafeRelease(_rasterstate);
    SafeRelease(_depthstate);
    SafeRelease(_rtview);
    SafeRelease(_dsview);
    SafeRelease(_swapchain);
    SafeRelease(_context);
#if defined(DEBUG) || defined(_DEBUG)
    ID3D11Debug* pDebug = nullptr;
    HRESULT hr = _device->QueryInterface(__uuidof(ID3D11Debug), (void**)&pDebug);
    if(SUCCEEDED(hr) && pDebug)
        pDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
    SafeRelease(pDebug);
#endif
    SafeRelease(_device);
}

void rendersys_d3d11::setup_pipeline_state()
{
    assert(_context && _rtview && _dsview);
    _context->OMSetRenderTargets(1, &_rtview, _dsview);
    assert(_swapchain);
    DXGI_SWAP_CHAIN_DESC sd;
    memset(&sd, 0, sizeof(sd));
    _swapchain->GetDesc(&sd);
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)sd.BufferDesc.Width;
    vp.Height = (FLOAT)sd.BufferDesc.Height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    _context->RSSetViewports(1, &vp);
    enable_alpha_blend(false);
    enable_depth(true);
    if(_msaa)
        _context->RSSetState(_rasterstate);
}

render_blob* rendersys_d3d11::compile_shader_from_file(const gchar* file, const gchar* entry, const gchar* sm, render_include* inc)
{
    HRESULT hr = S_OK;
    DWORD flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined (DEBUG) || defined (_DEBUG)
    flags |= D3DCOMPILE_DEBUG;
#endif
    com_ptr<render_blob> err_blob;
    com_ptr<render_blob> shader_blob;
    _string<char> sbentry, sbsmodel;
    sbentry.from(entry);
    sbsmodel.from(sm);
    _string<wchar> mbfile;
    mbfile.from(file);
    HMODULE hmod = LoadLibraryA("d3dcompiler_47.dll");
    if(!hmod) {
        assert(!"LoadLibrary(\"d3dcompiler_47.dll\") failed.");
        return nullptr;
    }
    /* use dynamic link to avoid d3dcompiler.dll missing failure. */
    typedef HRESULT(__stdcall* fnD3DCompileFromFile)(LPCWSTR pFileName,
        const D3D_SHADER_MACRO* pDefines,
        ID3DInclude* pInclude,
        LPCSTR pEntrypoint,
        LPCSTR pTarget,
        UINT Flags1,
        UINT Flags2,
        ID3DBlob** ppCode,
        ID3DBlob** ppErrorMsgs
        );
    fnD3DCompileFromFile fn = (fnD3DCompileFromFile)GetProcAddress(hmod, "D3DCompileFromFile");
    if(!fn) {
        assert(!"GetProcAddress(\"D3DCompileFromFile\") failed.");
        FreeLibrary(hmod);
        return nullptr;
    }
    hr = fn(mbfile.c_str(), 0, inc, sbentry.c_str(), sbsmodel.c_str(), flags, 0, &shader_blob, &err_blob);
    if(FAILED(hr)) {
        if(err_blob.get())
            OutputDebugStringA((char*)err_blob->GetBufferPointer());
        FreeLibrary(hmod);
        return nullptr;
    }
    FreeLibrary(hmod);
    return shader_blob.detach();
}

render_blob* rendersys_d3d11::compile_shader_from_memory(const char* src, int len, const gchar* name, const gchar* entry, const gchar* sm, render_include* inc)
{
    HRESULT hr = S_OK;
    DWORD flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined (DEBUG) || defined (_DEBUG)
    flags |= D3DCOMPILE_DEBUG;
#endif
    com_ptr<render_blob> err_blob;
    com_ptr<render_blob> shader_blob;
    _string<char> sbname, sbentry, sbsmodel;
    sbentry.from(entry);
    sbsmodel.from(sm);
    sbname.from(name);
    HMODULE hmod = LoadLibraryA("d3dcompiler_47.dll");
    if(!hmod) {
        assert(!"LoadLibrary(\"d3dcompiler_47.dll\") failed.");
        return nullptr;
    }
    /* use dynamic link to avoid d3dcompiler.dll missing failure. */
    pD3DCompile fn = (pD3DCompile)GetProcAddress(hmod, "D3DCompile");
    if(!fn) {
        assert(!"GetProcAddress(\"D3DCompile\") failed.");
        FreeLibrary(hmod);
        return nullptr;
    }
    hr = fn(src, len, sbname.c_str(), 0, inc, sbentry.c_str(), sbsmodel.c_str(), flags, 0, &shader_blob, &err_blob);
    if(FAILED(hr)) {
        if(err_blob.get())
            OutputDebugStringA((char*)err_blob->GetBufferPointer());
        FreeLibrary(hmod);
        return nullptr;
    }
    FreeLibrary(hmod);
    return shader_blob.detach();
}

vertex_shader* rendersys_d3d11::create_vertex_shader(const void* ptr, size_t len)
{
    assert(_device);
    vertex_shader* shader = nullptr;
    return FAILED(_device->CreateVertexShader(ptr, len, 0, &shader)) ? nullptr : shader;
}

pixel_shader* rendersys_d3d11::create_pixel_shader(const void* ptr, size_t len)
{
    pixel_shader* shader = nullptr;
    assert(_device);
    return FAILED(_device->CreatePixelShader(ptr, len, 0, &shader)) ? nullptr : shader;
}

compute_shader* rendersys_d3d11::create_compute_shader(const void* ptr, size_t len)
{
    compute_shader* shader = nullptr;
    assert(_device);
    return FAILED(_device->CreateComputeShader(ptr, len, 0, &shader)) ? nullptr : shader;
}

geometry_shader* rendersys_d3d11::create_geometry_shader(const void* ptr, size_t len)
{
    geometry_shader* shader = nullptr;
    assert(_device);
    return FAILED(_device->CreateGeometryShader(ptr, len, 0, &shader)) ? nullptr : shader;
}

hull_shader* rendersys_d3d11::create_hull_shader(const void* ptr, size_t len)
{
    hull_shader* shader = nullptr;
    assert(_device);
    return FAILED(_device->CreateHullShader(ptr, len, 0, &shader)) ? nullptr : shader;
}

domain_shader* rendersys_d3d11::create_domain_shader(const void* ptr, size_t len)
{
    domain_shader* shader = nullptr;
    assert(_device);
    return FAILED(_device->CreateDomainShader(ptr, len, 0, &shader)) ? 0 : shader;
}

vertex_format* rendersys_d3d11::create_vertex_format(const void* ptr, size_t len, vertex_format_desc desc[], uint n)
{
    vertex_format* format = nullptr;
    assert(_device);
    return FAILED(_device->CreateInputLayout(desc, n, ptr, len, &format)) ? nullptr : format;
}

render_vertex_buffer* rendersys_d3d11::create_vertex_buffer(uint stride, uint count, bool read, bool write, uint usage, const void* ptr)
{
    D3D11_BUFFER_DESC desc;
    memset(&desc, 0, sizeof(desc));
    desc.Usage = (D3D11_USAGE)usage;
    desc.ByteWidth = stride * count;
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    desc.CPUAccessFlags = 0;
    if(read != false)
        desc.CPUAccessFlags |= D3D11_CPU_ACCESS_READ;
    if(write != false)
        desc.CPUAccessFlags |= D3D11_CPU_ACCESS_WRITE;
    D3D11_SUBRESOURCE_DATA* psd = nullptr, sd;
    if(ptr) {
        memset(&sd, 0, sizeof(sd));
        sd.pSysMem = ptr;
        psd = &sd;
    }
    vertex_buffer* vb = nullptr;
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
    D3D11_SUBRESOURCE_DATA* psd = nullptr, sd;
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
    D3D11_SUBRESOURCE_DATA* psd = nullptr, sd;
    if(ptr) {
        memset(&sd, 0, sizeof(sd));
        sd.pSysMem = ptr;
        psd = &sd;
    }
    constant_buffer* cb;
    return FAILED(_device->CreateBuffer(&desc, psd, &cb)) ? 0 : cb;
}

shader_resource_view* rendersys_d3d11::create_shader_resource_view(render_resource* res)
{
    assert(res);
    shader_resource_view* p = nullptr;
    HRESULT hr = _device->CreateShaderResourceView(res, nullptr, &p);
    if(FAILED(hr) || !p)
        return nullptr;
    return p;
}

depth_stencil_view* rendersys_d3d11::create_depth_stencil_view(render_resource* res)
{
    assert(res);
    depth_stencil_view* p = nullptr;
    HRESULT hr = _device->CreateDepthStencilView(res, nullptr, &p);
    if(FAILED(hr) || !p)
        return nullptr;
    return p;
}

unordered_access_view* rendersys_d3d11::create_unordered_access_view(render_resource* res)
{
    assert(res);
    unordered_access_view* p = nullptr;
    HRESULT hr = _device->CreateUnorderedAccessView(res, nullptr, &p);
    if(FAILED(hr) || !p)
        return nullptr;
    return p;
}

render_sampler_state* rendersys_d3d11::create_sampler_state(sampler_state_filter filter)
{
    D3D11_SAMPLER_DESC desc;
    desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    desc.MipLODBias = 0.f;
    desc.MaxAnisotropy = 1;
    desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    desc.BorderColor[0] = 0;
    desc.BorderColor[1] = 0;
    desc.BorderColor[2] = 0;
    desc.BorderColor[3] = 0;
    desc.MinLOD = 0.f;
    desc.MaxLOD = D3D11_FLOAT32_MAX;
    switch(filter)
    {
    case ssf_point:
        desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
        break;
    case ssf_anisotropic:
        desc.Filter = D3D11_FILTER_ANISOTROPIC;
        desc.MaxAnisotropy = 16;
        break;
    case ssf_linear:
    default:
        desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        break;
    }
    ID3D11SamplerState* sstate = nullptr;
    HRESULT hr = _device->CreateSamplerState(&desc, &sstate);
    if(FAILED(hr))
        return nullptr;
    assert(sstate);
    return sstate;
}

render_texture2d* rendersys_d3d11::create_texture2d(const image& img, uint mips, uint usage, uint bindflags, uint cpuflags, uint miscflags)
{
    D3D11_TEXTURE2D_DESC desc;
    desc.Width = img.get_width();
    desc.Height = img.get_height();
    desc.MipLevels = mips;
    desc.ArraySize = 1;
    switch(img.get_format())
    {
    case image::fmt_rgba:
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        break;
    default:
        assert(!"unexpected format.");
        break;
    }
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = (D3D11_USAGE)usage;
    desc.BindFlags = bindflags;
    desc.CPUAccessFlags = cpuflags;
    desc.MiscFlags = miscflags;
    D3D11_SUBRESOURCE_DATA subdata;
    subdata.pSysMem = img.get_data(0, 0);
    subdata.SysMemPitch = img.get_bytes_per_line();
    subdata.SysMemSlicePitch = 0;
    ID3D11Texture2D* tex = nullptr;
    if(FAILED(_device->CreateTexture2D(&desc, &subdata, &tex)))
        return nullptr;
    assert(tex);
    return tex;
}

render_texture2d* rendersys_d3d11::create_texture2d(int width, int height, uint format, uint mips, uint usage, uint bindflags, uint cpuflags, uint miscflags)
{
    D3D11_TEXTURE2D_DESC desc;
    desc.Width = (uint)width;
    desc.Height = (uint)height;
    desc.MipLevels = mips;
    desc.ArraySize = 1;
    desc.Format = (DXGI_FORMAT)format;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = (D3D11_USAGE)usage;
    desc.BindFlags = bindflags;
    desc.CPUAccessFlags = cpuflags;
    desc.MiscFlags = miscflags;
    ID3D11Texture2D* tex = nullptr;
    if(FAILED(_device->CreateTexture2D(&desc, nullptr, &tex)))
        return nullptr;
    assert(tex);
    return tex;
}

void rendersys_d3d11::load_with_mips(texture2d* tex, const image& img)
{
    assert(tex);
    com_ptr<ID3D11ShaderResourceView> cpsrv;
    if(FAILED(_device->CreateShaderResourceView(tex, nullptr, &cpsrv)))
        return;
    UINT row_pitch = (UINT)img.get_bytes_per_line();
    UINT img_size = row_pitch * (UINT)img.get_height();
    _context->UpdateSubresource(tex, 0, nullptr, img.get_data(0, 0), row_pitch, img_size);
    _context->GenerateMips(cpsrv.get());
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
    _context->ClearRenderTargetView(_rtview, _bkcr);
    _context->ClearDepthStencilView(_dsview, D3D11_CLEAR_DEPTH, 1.f, 0);
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

void rendersys_d3d11::set_constant_buffer(uint slot, constant_buffer* cb, shader_type st)
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

void rendersys_d3d11::set_sampler_state(uint slot, sampler_state* sstate, shader_type st)
{
    assert(sstate);
    switch(st)
    {
    case st_vertex_shader:
        _context->VSSetSamplers(slot, 1, &sstate);
        break;
    case st_pixel_shader:
        _context->PSSetSamplers(slot, 1, &sstate);
        break;
    default:
        assert(!"unknown shader type.");
    }
}

void rendersys_d3d11::set_shader_resource(uint slot, shader_resource_view* srv, shader_type st)
{
    assert(srv);
    switch(st)
    {
    case st_vertex_shader:
        _context->VSSetShaderResources(slot, 1, &srv);
        break;
    case st_pixel_shader:
        _context->PSSetShaderResources(slot, 1, &srv);
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

void rendersys_d3d11::capture_screen(image& img, const rectf& rc, int buff_id)
{
    texture2d* tex = create_texture2d((int)ceil(rc.width()), (int)ceil(rc.height()), DXGI_FORMAT_R8G8B8A8_UNORM, 1, D3D11_USAGE_DEFAULT, D3D11_BIND_UNORDERED_ACCESS, 0, 0);
    assert(tex);
    com_ptr<ID3D11Texture2D> buffer;
    assert(_swapchain);
    _swapchain->GetBuffer(buff_id, __uuidof(ID3D11Texture2D), (void**)&buffer);
    if(!buffer.get()) {
        assert(!"get swap chain buffer failed.");
        return;
    }
    textureop texop(this);
    D3D11_TEXTURE2D_DESC desc;
    buffer->GetDesc(&desc);
    if(desc.SampleDesc.Count <= 1)
        texop.copy_rect(tex, buffer.get(), 0, 0, (int)floorf(rc.left), (int)floorf(rc.top), (int)ceilf(rc.width()), (int)ceilf(rc.height()));
    else {      /* need resolve */
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.CPUAccessFlags = 0;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        com_ptr<ID3D11Texture2D> tmp;
        _device->CreateTexture2D(&desc, nullptr, &tmp);
        assert(tmp.get());
        _context->ResolveSubresource(tmp.get(), 0, buffer.get(), 0, DXGI_FORMAT_R8G8B8A8_UNORM);
        texop.copy_rect(tex, tmp.get(), 0, 0, (int)floorf(rc.left), (int)floorf(rc.top), (int)ceilf(rc.width()), (int)ceilf(rc.height()));
    }
    texop.convert_to_image(img, tex);
    tex->Release();
}

void rendersys_d3d11::enable_alpha_blend(bool b)
{
    _context->OMSetBlendState(b ? _blendstate : nullptr, 0, 0xffffffff);
}

void rendersys_d3d11::enable_depth(bool b)
{
    _context->OMSetDepthStencilState(!b ? _depthstate : nullptr, 0);
}

void rendersys_d3d11::install_configs(const configs& cfg)
{
    _vsync = is_vsync_enabled(cfg);
    _fullscreen = is_full_screen(cfg);
    _msaa = is_MSAA_enabled(cfg);
    if(_msaa)
        _msaa_x = convert_msaa_x(get_MSAA_sampler_count(cfg));
}

void release_vertex_buffer(render_vertex_buffer* buf) { if(buf) buf->Release(); }
void release_index_buffer(render_index_buffer* buf) { if(buf) buf->Release(); }
void release_constant_buffer(render_constant_buffer* buf) { if(buf) buf->Release(); }
void release_texture2d(render_texture2d* tex) { if(tex) tex->Release(); }

__ariel_end__
