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

#ifndef wsysdrvd3d11_201c78b1_b2a1_4c13_9151_81f6cc09ffac_h
#define wsysdrvd3d11_201c78b1_b2a1_4c13_9151_81f6cc09ffac_h

#include <windows.h>
#include <pink/widget.h>
#include <ariel/rendersysd3d11.h>

__pink_begin__

class wsysdrvd3d11:
    public wsys_driver
{
public:
    wsysdrvd3d11();
    ~wsysdrvd3d11();
    void initialize(wsys_notify* ptr, const rect& rc) override;
    void close() override;

public:
    virtual int run();

protected:
    bool setup();
    void cleanup();
    static LRESULT __stdcall wndproc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

protected:
    HWND                    _hwnd;
    HINSTANCE               _hinstance;
    wsys_notify*            _notify;
    gchar                   _clsname[128];
    D3D_DRIVER_TYPE         _drvtype;
    IDXGISwapChain*         _swapchain;
    ID3D11Device*           _d11dev;
    D3D_FEATURE_LEVEL       _level;
    ID3D11DeviceContext*    _context;
    ID3D11RenderTargetView* _rtview;
};

__pink_end__

#endif
