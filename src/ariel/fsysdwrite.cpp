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

#include <d3d10_1.h>
#include <d3d11.h>
#include <d2d1.h>
#include <dxgi.h>
#include <dwrite.h>
#include <ariel/scene.h>
#include <ariel/fsysdwrite.h>
#include <ariel/textureop.h>

__ariel_begin__

static DWRITE_FONT_WEIGHT translate_dwrite_font_weight(int weight)
{
    int w = weight % 10 * 100 + 100;
    if(w == 1000)
        w = 950;
    return (DWRITE_FONT_WEIGHT)w;
}

static DWRITE_FONT_STYLE translate_dwrite_font_style(uint mask)
{
    return (mask & font::ftm_italic) ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL;
}

fsys_dwrite::fsys_dwrite()
{
    _current_font = nullptr;
}

fsys_dwrite::~fsys_dwrite()
{
    destroy_font_map();
}

void fsys_dwrite::initialize()
{
    auto* rsys = scene::get_singleton_ptr()->get_rendersys();
    assert(rsys);
    _dev11 = static_cast<rendersys_d3d11*>(rsys)->get_device();
    com_ptr<IDXGIFactory1> spfactory;
    HRESULT hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&spfactory);
    verify(SUCCEEDED(hr));
    com_ptr<IDXGIAdapter1> spadapter;
    hr = spfactory->EnumAdapters1(0, &spadapter);
    verify(SUCCEEDED(hr));
    UINT flags = D3D10_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(DEBUG) || defined(_DEBUG)
    flags |= D3D10_CREATE_DEVICE_DEBUG;
#endif
    hr = D3D10CreateDevice1(spadapter.get(), D3D10_DRIVER_TYPE_HARDWARE, NULL, flags, D3D10_FEATURE_LEVEL_9_3, D3D10_1_SDK_VERSION, &_dev101);
    verify(SUCCEEDED(hr));
    com_ptr<IUnknown> spunkdwfactory;
    hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), &spunkdwfactory);
    verify(SUCCEEDED(hr));
    spunkdwfactory->QueryInterface(__uuidof(IDWriteFactory), (void**)&_dwfactory);
    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory), (void**)&_d2dfactory);
    verify(SUCCEEDED(hr));
}

void fsys_dwrite::set_font(const font& ft)
{
    if(ft.sysfont) {
        _current_font = (IDWriteTextFormat*)ft.sysfont;
        return;
    }
    auto f = _font_map.find(ft);
    if(f != _font_map.end()) {
        ft.sysfont = (uint)f->second;
        _current_font = f->second;
        return;
    }
    gchar locale_name[LOCALE_NAME_MAX_LENGTH];
    int ret = GetUserDefaultLocaleName(locale_name, LOCALE_NAME_MAX_LENGTH);
    if(ret <= 0)
        strtool::copy(locale_name, _countof(_t("en-US")), _t("en-US"));
    auto font_weight = translate_dwrite_font_weight(ft.weight);
    auto font_style = translate_dwrite_font_style(ft.mask);
    float font_size = (float)ft.size * 96.f / 72.f;
    com_ptr<IDWriteTextFormat> spformat;
    HRESULT hr = _dwfactory->CreateTextFormat(ft.name.c_str(), nullptr, font_weight, font_style,
        DWRITE_FONT_STRETCH_NORMAL, (FLOAT)font_size, locale_name, &spformat
        );
    verify(SUCCEEDED(hr));
    ft.sysfont = (uint)spformat.get();
    _current_font = spformat.get();
    _font_map.emplace(ft, spformat.detach());
}

bool fsys_dwrite::query_size(const gchar* str, int& w, int& h, int len)
{
    if(!str) {
        w = 0, h = 0;
        return false;
    }
    assert(len <= strtool::length(str));
    if(len <= 0)
        len = strtool::length(str);
    assert(_dwfactory);
#if !defined(UNICODE) && !defined(_UNICODE)
    _string<wchar> wstr;
    wstr.from(str, len);
    const wchar* ws = wstr.c_str();
#else
    const wchar* ws = str;
#endif
    com_ptr<IDWriteTextLayout> splayout;
    _dwfactory->CreateTextLayout(ws, len, _current_font, FLT_MAX, FLT_MAX, &splayout);
    assert(splayout);
    splayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
    splayout->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
    DWRITE_TEXT_METRICS metrics;
    HRESULT hr = splayout->GetMetrics(&metrics);
    if(FAILED(hr))
        return false;
    w = (int)ceil(metrics.width);
    h = (int)ceil(metrics.height);
    return true;
}

bool fsys_dwrite::create_text_texture(texture2d** tex, const gchar* str, int margin, const color& cr, int len)
{
    if(!str || !tex)
        return false;
    if(len < 0)
        len = strtool::length(str);
    int w, h;
    query_size(str, w, h, len);
    w += (margin * 2);
    h += (margin * 2);
    D3D11_TEXTURE2D_DESC stdesc;
    memset(&stdesc, 0, sizeof(stdesc));
    stdesc.Width = w;
    stdesc.Height = h;
    stdesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    stdesc.MipLevels = 1;
    stdesc.ArraySize = 1;
    stdesc.SampleDesc.Count = 1;
    stdesc.Usage = D3D11_USAGE_DEFAULT;
    stdesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    stdesc.MiscFlags = D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX;
    com_ptr<ID3D11Texture2D> sptex11;
    HRESULT hr = _dev11->CreateTexture2D(&stdesc, nullptr, &sptex11);
    if(FAILED(hr))
        return false;
    com_ptr<IDXGIKeyedMutex> kmutex11;
    sptex11->QueryInterface(__uuidof(IDXGIKeyedMutex), (void**)&kmutex11);
    com_ptr<IDXGIResource> sharedres10;
    sptex11->QueryInterface(__uuidof(IDXGIResource), (void**)&sharedres10);
    HANDLE sharedhandle10;
    sharedres10->GetSharedHandle(&sharedhandle10);
    com_ptr<IDXGISurface1> spsurface101;
    _dev101->OpenSharedResource(sharedhandle10, __uuidof(IDXGISurface1), (void**)&spsurface101);
    com_ptr<IDXGIKeyedMutex> kmutex101;
    spsurface101->QueryInterface(__uuidof(IDXGIKeyedMutex), (void**)&kmutex101);
    D2D1_RENDER_TARGET_PROPERTIES rtprops;
    memset(&rtprops, 0, sizeof(rtprops));
    rtprops.type = D2D1_RENDER_TARGET_TYPE_HARDWARE;
    rtprops.pixelFormat = D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED);
    com_ptr<IDXGISurface> spsurface10;
    spsurface101->QueryInterface(__uuidof(IDXGISurface), (void**)&spsurface10);
    com_ptr<ID2D1RenderTarget> d2drt;
    hr = _d2dfactory->CreateDxgiSurfaceRenderTarget(spsurface10.get(), &rtprops, &d2drt);
    if(FAILED(hr))
        return false;
    com_ptr<ID2D1SolidColorBrush> spbrush;
    D2D1_COLOR_F crf = { (float)cr.red / 255.f, (float)cr.green / 255.f, (float)cr.blue / 255.f, (float)cr.alpha / 255.f };
    d2drt->CreateSolidColorBrush(crf, &spbrush);
    /* rendering */
    kmutex11->ReleaseSync(0);
    kmutex101->AcquireSync(0, INFINITE);
    d2drt->BeginDraw();
    D2D1_RECT_F rc = D2D1::RectF((float)margin, (float)margin, (float)(w - margin), (float)(h - margin));
    d2drt->DrawText(str, len, _current_font, &rc, spbrush.get());
    d2drt->EndDraw();
    kmutex101->ReleaseSync(1);
    kmutex11->AcquireSync(1, INFINITE);
    if(tex) {
        auto* rsys = scene::get_singleton_ptr()->get_rendersys();
        assert(rsys);
        *tex = textureop(rsys).convert_from_premultiplied(sptex11.get());
        return *tex ? true : false;
    }
    return false;
}

bool fsys_dwrite::create_text_image(image& img, const gchar* str, int x, int y, const color& cr, int len)
{
    assert(!"Deprecated function due to low performance.");
    return false;
}

void fsys_dwrite::draw(image& img, const gchar* str, int x, int y, const color& cr, int len)
{
    assert(!"Deprecated function due to low performance.");
}

void fsys_dwrite::destroy_font_map()
{
    for(auto& p : _font_map)
        p.second->Release();
    _font_map.clear();
    _current_font = nullptr;
}

__ariel_end__
