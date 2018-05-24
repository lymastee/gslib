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
#include <pink/image.h>
#include <pink/widget.h>
#include <pink/imageio.h>

__pink_begin__

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
    int pos = _bytes_per_line * y + x;
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

__pink_end__
