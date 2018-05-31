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

#include <assert.h>
#include <ariel/image.h>
#include <ariel/widget.h>
#include <ariel/imageio.h>

__ariel_begin__

image::image()
{
    _width = _height = 0;
    _depth = 0;
    _color_bytes = 0;
    _bytes_per_line = 0;
    _data = 0;
    _xdpi = _ydpi = 96;
    _is_alpha_channel_valid = false;
}

bool image::is_valid() const
{
    if(_data) {
        assert((_width != 0) && (_height != 0));
        assert(_color_bytes != 0);
        assert(_bytes_per_line != 0);
        return true;
    }
    return false;
}

bool image::create(image_format fmt, int w, int h)
{
    if(!w || !h)
        return false;
    if(is_valid())
        destroy();
    _width = w;
    _height = h;
    _format = fmt;
    switch(fmt)
    {
    case fmt_gray:
        _depth = 8;
        break;
    case fmt_rgba:
        _depth = 32;
        break;
    default:
        return false;
    }
    _xdpi = _ydpi = 96;
    _bytes_per_line = (int)(((uint)(_depth * w + 31)) >> 5 << 2);
    _color_bytes = _bytes_per_line * h;
    _data = new byte[_color_bytes];
    return true;
}

void image::destroy()
{
    if(_data) {
        delete [] _data;
        _data = 0;
    }
    _width = _height = 0;
    _depth = 0;
    _color_bytes = 0;
    _bytes_per_line = 0;
}

void image::init(const color& cr)
{
    if(!_data)
        return;
    assert(_format == fmt_rgba && _depth == 32);
    memset(_data, cr.data(), _color_bytes);
}

byte* image::get_data(int x, int y) const
{
    assert(is_valid());
    if((x >= _width) || (y >= _height)) {
        assert(!"invalid position.");
        return 0;
    }
    int pos = 0;
    switch(_format)
    {
    case fmt_gray:
        assert(_depth == 8);
        pos = _bytes_per_line * y + x;
        break;
    case fmt_rgba:
        assert(_depth == 32);
        pos = _bytes_per_line * y + x * 4;
        break;
    }
    return _data + pos;
}

bool image::load(const string& filepath)
{
    return imageio::read_image(*this, filepath);
}

bool image::save(const string& filepath) const
{
    return imageio::save_image(*this, filepath);
}

void image::clear(byte b, const rect* rc)
{
    assert(_format == fmt_gray);
    rect r;
    if(!rc)
        r = rect(0, 0, get_width(), get_height());
    else {
        r.top = gs_max(0, rc->top);
        r.left = gs_max(0, rc->left);
        r.bottom = gs_min(get_height(), rc->bottom);
        r.right = gs_min(get_width(), rc->right);
    }
    for(int j = r.top; j < r.bottom; j ++) {
        byte* p = get_data(r.left, j);
        assert(p);
        memset(p, b, r.width());
    }
}

void image::clear(const color& cr, const rect* rc)
{
    assert(_format == fmt_rgba);
    rect r;
    if(!rc)
        r = rect(0, 0, get_width(), get_height());
    else {
        r.top = gs_max(0, rc->top);
        r.left = gs_max(0, rc->left);
        r.bottom = gs_min(get_height(), rc->bottom);
        r.right = gs_min(get_width(), rc->right);
    }
    for(int j = r.top; j < r.bottom; j ++) {
        for(int i = r.left; i < r.right; i ++) {
            color* p = reinterpret_cast<color*>(get_data(i, j));
            assert(p);
            *p = cr;
        }
    }
}

void image::copy(const image& img)
{
    if(!img.is_valid())
        return;
    copy(img, 0, 0, img.get_width(), img.get_height(), 0, 0);
}

void image::copy(const image& img, int x, int y, int cx, int cy, int sx, int sy)
{
    if(!img.is_valid() || !cx || !cy || (img.get_format() != get_format()))
        return;
    if(cx < 0 || x + cx > get_width())
        cx = get_width() - x;
    if(cy < 0 || y + cy > get_height())
        cy = get_height() - y;
    if(sx + cx > img.get_width())
        cx = img.get_width() - sx;
    if(sy + cy > img.get_height())
        cy = img.get_height() - sy;
    int sp = 1;
    if(_format == fmt_rgba)
        sp = 4;
    int size = cx * sp;
    for(int j = 0; j < cy; j ++) {
        const byte* src = img.get_data(sx, sy + j);
        byte* dest = get_data(x, y + j);
        assert(src && dest);
        memcpy(dest, src, size);
    }
}

static void adjust_brightness(color& original, float scale)
{
    assert(scale >= 0 && scale < 2.0f);
    if(scale == 1.0f);
    else if(scale > 1.0f) {
        scale -= 1.0f;
        int r = 0xff - original.red;
        float rf = (float)r * scale;
        original.red += (int)rf;
        r = 0xff - original.green;
        rf = (float)r * scale;
        original.green += (int)rf;
        r = 0xff - original.blue;
        rf = (float)r * scale;
        original.blue += (int)rf;
    }
    else {
        float r = (float)original.red * scale;
        original.red = (int)r;
        r = (float)original.green * scale;
        original.green = (int)r;
        r = (float)original.blue * scale;
        original.blue = (int)r;
    }
}

void image::set_brightness(const image& img, float s)
{
    if(_format != fmt_rgba)
        return;
    int w = get_width(), h = get_height();
    if(img.get_width() != w || img.get_height() != h)
        return;
    for(int j = 0; j < h; j ++) {
        for(int i = 0; i < w; i ++) {
            color* dest = reinterpret_cast<color*>(get_data(i, j));
            const color* src = reinterpret_cast<const color*>(img.get_data(i, j));
            assert(dest && src);
            *dest = *src;
            adjust_brightness(*dest, s);
        }
    }
}

void image::set_gray(const image& img)
{
    if(_format != fmt_rgba)
        return;
    int w = get_width(), h = get_height();
    if(img.get_width() != w || img.get_height() != h)
        return;
    for(int j = 0; j < h; j ++) {
        for(int i = 0; i < w; i ++) {
            color* dest = reinterpret_cast<color*>(get_data(i, j));
            const color* src = reinterpret_cast<const color*>(img.get_data(i, j));
            assert(dest && src);
            int av = src->red + src->green + src->blue;
            av /= 3;
            dest->red = dest->green = dest->blue = av;
            dest->alpha = src->alpha;
        }
    }
}

__ariel_end__
