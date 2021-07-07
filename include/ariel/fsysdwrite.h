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

#ifndef fsysdwrite_c5ab9cfc_e66a_40b6_8ea1_29b3bde4f5e3_h
#define fsysdwrite_c5ab9cfc_e66a_40b6_8ea1_29b3bde4f5e3_h

#include <d3d11.h>
#include <d3d10_1.h>
#include <dxgi.h>
#include <d2d1.h>
#include <dwrite.h>
#include <gslib/std.h>
#include <ariel/sysop.h>
#include <ariel/rendersysd3d11.h>

__ariel_begin__

typedef unordered_map<font, IDWriteTextFormat*> dwrite_font_map;

class fsys_dwrite:
    public fontsys
{
public:
    fsys_dwrite();
    virtual ~fsys_dwrite();
    virtual void initialize() override;
    virtual void set_font(const font& f) override;
    virtual bool query_size(const gchar* str, int& w, int& h, int len = -1) override;
    virtual bool create_text_image(image& img, const gchar* str, int x, int y, const color& cr, int len = -1) override;
    virtual bool create_text_texture(texture2d** tex, const gchar* str, int margin, const color& cr, int len = -1) override;
    virtual void draw(image& img, const gchar* str, int x, int y, const color& cr, int len = -1) override;

protected:
    dwrite_font_map                     _font_map;
    IDWriteTextFormat*                  _current_font;
    com_ptr<ID3D11Device>               _dev11;
    com_ptr<ID3D10Device1>              _dev101;
    com_ptr<IDWriteFactory>             _dwfactory;
    com_ptr<ID2D1Factory>               _d2dfactory;

protected:
    void destroy_font_map();
};

__ariel_end__

#endif
