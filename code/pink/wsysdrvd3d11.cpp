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

#include <d3d11.h>
#include <d3dx11.h>
#include <pink/wsysdrvd3d11.h>

#ifndef SafeRelease
#define SafeRelease(ptr) if(ptr) { \
    ptr->Release(); \
    ptr = 0; \
}
#endif

__pink_begin__

wsysdrvd3d11::wsysdrvd3d11()
{
    _hwnd = 0;
    _hinstance = 0;
    _notify = 0;
    _drvtype = D3D_DRIVER_TYPE_NULL;
    _swapchain = 0;
    _d11dev = 0;
    _level = D3D_FEATURE_LEVEL_11_0;
    _context = 0;
    _rtview = 0;
    strtool::copy(_clsname, _countof(_clsname), _t("wsysdrvd3d11"));
}

wsysdrvd3d11::~wsysdrvd3d11()
{
    cleanup();
}

void wsysdrvd3d11::initialize(wsys_notify* ptr, const rect& rc)
{
    if(_hwnd) {
        assert(!"window already existed.");
        return;
    }
    HINSTANCE hInst = _hinstance;

    /* register class */
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = wsysdrvd3d11::wndproc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInst;
    wcex.hIcon = LoadIcon(hInst, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName = 0;
    wcex.lpszClassName = _clsname;
    wcex.hIconSm = LoadIcon(hInst, IDI_WINLOGO);
    if(!RegisterClassEx(&wcex)) {
        assert(!"register class failed.");
        return;
    }

    /* create window */
    rect rc1 = rc;
    AdjustWindowRect((LPRECT)&rc1, WS_OVERLAPPEDWINDOW, FALSE);
    _hwnd = CreateWindow(_clsname, _t(""), WS_OVERLAPPEDWINDOW, 
        rc1.left, rc1.top, rc1.width(), rc1.height(),
        0, 0, hInst, 0
        );
    assert(_hwnd);
    if(!setup()) {
        assert(!"create d3d device failed.");
        return;
    }

    /* notify created */
    GetClientRect(_hwnd, (LPRECT)&rc1);
    _notify->on_create(this, rc1);

    /* initialize */
    ShowWindow(_hwnd, SW_SHOW);
    UpdateWindow(_hwnd);
}

void wsysdrvd3d11::close()
{
}

int wsysdrvd3d11::run()
{
    return 0;
}

bool wsysdrvd3d11::setup()
{
    RECT rc;
    GetClientRect(_hwnd, (LPRECT)&rc);
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;
    UINT flags = 0;
    D3D_DRIVER_TYPE drvtype[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };
    UINT drvtypes = ARRAYSIZE(drvtype);
    D3D_FEATURE_LEVEL level[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };
    UINT levels = ARRAYSIZE(level);
    DXGI_SWAP_CHAIN_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.BufferCount = 1;
    desc.BufferDesc.Width = width;
    desc.BufferDesc.Height = height;
    desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.BufferDesc.RefreshRate.Numerator = 60;
    desc.BufferDesc.RefreshRate.Denominator = 1;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.OutputWindow = _hwnd;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Windowed = TRUE;

    bool suc = false;
    for(UINT i = 0; i < drvtypes; i ++) {
        _drvtype = drvtype[i];
        if(SUCCEEDED(D3D11CreateDeviceAndSwapChain(0, _drvtype, 0, flags, level, levels,
            D3D11_SDK_VERSION, &desc, &_swapchain, &_d11dev, &_level, &_context))) {
            suc = true;
            break;
        }
    }
    if(suc == false)
        return false;

    ID3D11Texture2D* texture = 0;
    if(FAILED(_swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&texture)))
        return false;

    suc = SUCCEEDED(_d11dev->CreateRenderTargetView(texture, 0, &_rtview));
    texture->Release();
    if(suc == false)
        return false;
    _context->OMSetRenderTargets(1, &_rtview, 0);

    D3D11_VIEWPORT vp;
    vp.Width = (float)width;
    vp.Height = (float)height;
    vp.MinDepth = 0.f;
    vp.MaxDepth = 1.f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    _context->RSSetViewports(1, &vp);

    return true;
}

void wsysdrvd3d11::cleanup()
{
    if(_context)
        _context->ClearState();
    SafeRelease(_rtview);
    SafeRelease(_swapchain);
    SafeRelease(_context);
    SafeRelease(_d11dev);
}

LRESULT __stdcall wsysdrvd3d11::wndproc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;
    switch(msg)
    {
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0;
}

__pink_end__
