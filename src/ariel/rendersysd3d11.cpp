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

#include <d3dcompiler.h>
#include <ariel/config.h>
#include <ariel/type.h>
#include <ariel/rendersysd3d11.h>

class FakeD3DBlob:
    public ID3DBlob
{
public:
    static FakeD3DBlob* create(void* p, SIZE_T s) { return new FakeD3DBlob(p, s); }
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject) override { return E_FAIL; }
    virtual ULONG STDMETHODCALLTYPE AddRef() override { return 1; }
    virtual ULONG STDMETHODCALLTYPE Release() override { delete this; return 0; }
    virtual LPVOID STDMETHODCALLTYPE GetBufferPointer() override { return m_ptr; }
    virtual SIZE_T STDMETHODCALLTYPE GetBufferSize() override { return m_size; }

private:
    void*           m_ptr;
    SIZE_T          m_size;

private:
    FakeD3DBlob(void* p, SIZE_T s): m_ptr(p), m_size(s) {}
};

template<class Interface>
inline void SafeRelease(Interface*& pInterface)
{
    if(pInterface) {
        pInterface->Release();
        pInterface = nullptr;
    }
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
    _drvtype = D3D_DRIVER_TYPE_NULL;
    _level = D3D_FEATURE_LEVEL_11_0;
    _device = nullptr;
    _context = nullptr;
    _swapchain = nullptr;
    _rtview = nullptr;
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
    com_ptr<ID3D11Texture2D> buffer;
    if(FAILED(_swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&buffer)))
        return false;
    bool fail = FAILED(_device->CreateRenderTargetView(buffer.get(), 0, &_rtview));
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
    com_ptr<ID3D11BlendState> blendstate;
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
    _context->OMSetBlendState(blendstate.get(), 0, 0xffffffff);
    return true;
}

void rendersys_d3d11::destroy()
{
    if(_context)
        _context->ClearState();
    SafeRelease(_rtview);
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

void rendersys_d3d11::begin_create_shader(create_shader_context& context, const void* buf, int size)
{
    context = FakeD3DBlob::create(const_cast<void*>(buf), size);
}

void rendersys_d3d11::begin_create_shader_from_file(create_shader_context& context, const gchar* file, const gchar* entry, const gchar* sm, render_include* inc)
{
    HRESULT hr = S_OK;
    DWORD flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined (DEBUG) || defined (_DEBUG)
    flags |= D3DCOMPILE_DEBUG;
#endif
    render_blob* err_blob = nullptr;
    _string<char> sbentry, sbsmodel;
    sbentry.from(entry);
    sbsmodel.from(sm);
    _string<wchar> mbfile;
    mbfile.from(file);
    HMODULE hmod = LoadLibraryA("d3dcompiler_47.dll");
    if(!hmod) {
        assert(!"LoadLibrary(\"d3dcompiler_47.dll\") failed.");
        return;
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
        return;
    }
    hr = fn(mbfile.c_str(), 0, inc, sbentry.c_str(), sbsmodel.c_str(), flags, 0, &context, &err_blob);
    if(FAILED(hr)) {
        if(err_blob)
            OutputDebugStringA((char*)err_blob->GetBufferPointer());
        SafeRelease(err_blob);
        FreeLibrary(hmod);
        return;
    }
    SafeRelease(err_blob);
    FreeLibrary(hmod);
}

void rendersys_d3d11::begin_create_shader_from_memory(create_shader_context& context, const char* src, int len, const gchar* name, const gchar* entry, const gchar* sm, render_include* inc)
{
    HRESULT hr = S_OK;
    DWORD flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined (DEBUG) || defined (_DEBUG)
    flags |= D3DCOMPILE_DEBUG;
#endif
    render_blob* err_blob = nullptr;
    _string<char> sbname, sbentry, sbsmodel;
    sbentry.from(entry);
    sbsmodel.from(sm);
    sbname.from(name);
    HMODULE hmod = LoadLibraryA("d3dcompiler_47.dll");
    if(!hmod) {
        assert(!"LoadLibrary(\"d3dcompiler_47.dll\") failed.");
        return;
    }
    /* use dynamic link to avoid d3dcompiler.dll missing failure. */
    pD3DCompile fn = (pD3DCompile)GetProcAddress(hmod, "D3DCompile");
    if(!fn) {
        assert(!"GetProcAddress(\"D3DCompile\") failed.");
        FreeLibrary(hmod);
        return;
    }
    hr = fn(src, len, sbname.c_str(), 0, inc, sbentry.c_str(), sbsmodel.c_str(), flags, 0, &context, &err_blob);
    if(FAILED(hr)) {
        if(err_blob)
            OutputDebugStringA((char*)err_blob->GetBufferPointer());
        SafeRelease(err_blob);
        FreeLibrary(hmod);
        return;
    }
    SafeRelease(err_blob);
    FreeLibrary(hmod);
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
    vertex_shader* shader = nullptr;
    return FAILED(_device->CreateVertexShader(context->GetBufferPointer(), context->GetBufferSize(), 0, &shader)) ? 0 : shader;
}

pixel_shader* rendersys_d3d11::create_pixel_shader(create_shader_context& context)
{
    if(!context)
        return 0;
    pixel_shader* shader = nullptr;
    assert(_device);
    return FAILED(_device->CreatePixelShader(context->GetBufferPointer(), context->GetBufferSize(), 0, &shader)) ? 0 : shader;
}

compute_shader* rendersys_d3d11::create_compute_shader(create_shader_context& context)
{
    if(!context)
        return 0;
    compute_shader* shader = nullptr;
    assert(_device);
    return FAILED(_device->CreateComputeShader(context->GetBufferPointer(), context->GetBufferSize(), 0, &shader)) ? 0 : shader;
}

geometry_shader* rendersys_d3d11::create_geometry_shader(create_shader_context& context)
{
    if(!context)
        return 0;
    geometry_shader* shader = nullptr;
    assert(_device);
    return FAILED(_device->CreateGeometryShader(context->GetBufferPointer(), context->GetBufferSize(), 0, &shader)) ? 0 : shader;
}

hull_shader* rendersys_d3d11::create_hull_shader(create_shader_context& context)
{
    if(!context)
        return 0;
    hull_shader* shader = nullptr;
    assert(_device);
    return FAILED(_device->CreateHullShader(context->GetBufferPointer(), context->GetBufferSize(), 0, &shader)) ? 0 : shader;
}

domain_shader* rendersys_d3d11::create_domain_shader(create_shader_context& context)
{
    if(!context)
        return 0;
    domain_shader* shader = nullptr;
    assert(_device);
    return FAILED(_device->CreateDomainShader(context->GetBufferPointer(), context->GetBufferSize(), 0, &shader)) ? 0 : shader;
}

vertex_format* rendersys_d3d11::create_vertex_format(create_shader_context& context, vertex_format_desc desc[], uint n)
{
    vertex_format* format = nullptr;
    assert(_device);
    return FAILED(_device->CreateInputLayout(desc, n, context->GetBufferPointer(), context->GetBufferSize(), &format)) ? 0 : format;
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

render_texture2d* rendersys_d3d11::create_texture2d(const image& img, uint mips, uint usage, uint bindflags, uint cpuflags)
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
    desc.MiscFlags = 0;
    if(mips > 1) {
        desc.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
        desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
    }
    D3D11_SUBRESOURCE_DATA subdata;
    subdata.pSysMem = img.get_data(0, 0);
    subdata.SysMemPitch = img.get_bytes_per_line();
    subdata.SysMemSlicePitch = 0;
    ID3D11Texture2D* tex = nullptr;
    if(FAILED(_device->CreateTexture2D(&desc, &subdata, &tex)))
        return nullptr;
    assert(tex);
    if(mips > 1) {
        com_ptr<ID3D11ShaderResourceView> spsrv;
        _device->CreateShaderResourceView(tex, nullptr, &spsrv);
        _context->GenerateMips(spsrv.get());
    }
    return tex;
}

render_texture2d* rendersys_d3d11::create_texture2d(int width, int height, uint format, uint mips, uint usage, uint bindflags, uint cpuflags)
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
    desc.MiscFlags = 0;
    if(mips > 1) {
        desc.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
        desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
    }
    ID3D11Texture2D* tex = nullptr;
    if(FAILED(_device->CreateTexture2D(&desc, nullptr, &tex)))
        return nullptr;
    assert(tex);
    if(mips > 1) {
        com_ptr<ID3D11ShaderResourceView> spsrv;
        _device->CreateShaderResourceView(tex, nullptr, &spsrv);
        _context->GenerateMips(spsrv.get());
    }
    return tex;
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

void rendersys_d3d11::install_configs(const configs& cfg)
{
    _vsync = is_vsync_enabled(cfg);
    _fullscreen = is_full_screen(cfg);
}

void release_vertex_buffer(render_vertex_buffer* buf) { if(buf) buf->Release(); }
void release_index_buffer(render_index_buffer* buf) { if(buf) buf->Release(); }
void release_constant_buffer(render_constant_buffer* buf) { if(buf) buf->Release(); }
void release_texture2d(render_texture2d* tex) { if(tex) tex->Release(); }

__ariel_end__
