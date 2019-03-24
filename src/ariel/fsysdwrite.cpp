/*
 * Copyright (c) 2016-2019 lymastee, All rights reserved.
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
#include <ariel/fsysdwrite.h>

__ariel_begin__

class fsys_dwrite_private
{
public:
    fsys_dwrite_private() {}

protected:
    com_ptr<ID3D10Device>           _dev10;
    com_ptr<IDXGIKeyedMutex>        _kmutex11;
    com_ptr<IDXGIKeyedMutex>        _kmutex10;
    com_ptr<ID2D1RenderTarget>      _d2drt;
    com_ptr<ID2D1SolidColorBrush>   _d2dbrush;
    com_ptr<ID3D11Texture2D>        _backbuffer11;
    com_ptr<ID3D11Texture2D>        _sharedtex11;
    com_ptr<ID3D11Buffer>           _d2dvertbuff;
    com_ptr<ID3D11Buffer>           _d2dindexbuff;
    com_ptr<ID3D11ShaderResourceView>   _d2dtex;
    com_ptr<IDWriteFactory>         _dwfactory;
    com_ptr<IDWriteTextFormat>      _textformat;
};

__ariel_end__
