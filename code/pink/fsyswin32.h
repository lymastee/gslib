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

#ifndef fsyswin32_02c06bb0_546a_4008_a296_7cd2c199bade_h
#define fsyswin32_02c06bb0_546a_4008_a296_7cd2c199bade_h

#include <windows.h>
#include <pink/widget.h>

__pink_begin__

struct font_pair
{
    font    _font;
    HFONT   _handle;
};

typedef deque<font_pair> fplist;

class fsys_win32:
    public fontsys
{
public:
    fsys_win32();
    virtual ~fsys_win32();
    virtual void initialize();
    virtual int set_font(const font& f, int idx = -1);
    virtual bool get_size(const gchar* str, int& w, int& h, int len = -1);
    virtual bool convert(image& img, const gchar* str, int x, int y, const pixel& p, int len = -1);
    virtual void draw(image& img, const gchar* str, int x, int y, const pixel& p, int len = -1);

protected:
    HDC         _container_dc;
    HFONT       _old_font;
    fplist      _fplist;
    int         _current;
};

__pink_end__

#endif