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

#ifndef fsyswin32_02c06bb0_546a_4008_a296_7cd2c199bade_h
#define fsyswin32_02c06bb0_546a_4008_a296_7cd2c199bade_h

#include <windows.h>
#include <gslib/std.h>
#include <ariel/sysop.h>

__ariel_begin__

typedef unordered_map<font, HFONT> font_map;

class fsys_win32:
    public fontsys
{
public:
    fsys_win32();
    virtual ~fsys_win32();
    virtual void initialize() override;
    virtual void set_font(const font& f) override;
    virtual bool query_size(const gchar* str, int& w, int& h, int len = -1) override;
    virtual bool create_text_image(image& img, const gchar* str, int x, int y, const color& cr, int len = -1) override;
    virtual bool create_text_texture(texture2d** tex, const gchar* str, int margin, const color& cr, int len = -1) override;
    virtual void draw(image& img, const gchar* str, int x, int y, const color& cr, int len = -1) override;

protected:
    HDC         _container_dc;
    HFONT       _old_font;
    font_map    _font_map;
};

__ariel_end__

#endif
