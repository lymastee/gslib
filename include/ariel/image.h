/*
 * Copyright (c) 2016-2020 lymastee, All rights reserved.
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
 
#ifndef image_a41c29de_bd9b_4bbd_9012_6dc956f524b3_h
#define image_a41c29de_bd9b_4bbd_9012_6dc956f524b3_h

#include <gslib/type.h>
#include <gslib/string.h>
#include <ariel/dirty.h>

__ariel_begin__

class image
{
public:
    enum image_format
    {
        fmt_gray,                   /* gray8 */
        fmt_rgba,                   /* rgba8888 */
    };

    friend class imageio;

public:
    image();
    ~image() { destroy(); }
    bool is_valid() const;
    image_format get_format() const { return _format; }
    int get_depth() const { return _depth; }
    bool create(image_format fmt, int w, int h);
    void destroy();
    void init(const color& cr);
    void enable_alpha_channel(bool b) { _is_alpha_channel_valid = b; }
    bool has_alpha() const { return _is_alpha_channel_valid; }
    int get_width() const { return _width; }
    int get_height() const { return _height; }
    int get_bytes_per_line() const { return _bytes_per_line; }
    int get_size() const { return _color_bytes; }
    void set_xdpi(int dpi) { _xdpi = dpi; }
    void set_ydpi(int dpi) { _ydpi = dpi; }
    int get_xdpi() const { return _xdpi; }
    int get_ydpi() const { return _ydpi; }
    byte* get_data(int x, int y) const;
    bool load(const string& filepath);
    bool load(const gchar* filepath, int len) { return load(string(filepath, len)); }
    bool save(const string& filepath) const;
    void clear(byte b, const rect* rc = nullptr);
    void clear(const color& cr, const rect* rc = nullptr);
    void copy(const image& img);
    void copy(const image& img, int x, int y, int cx, int cy, int sx, int sy);

protected:
    image_format        _format;
    int                 _width;
    int                 _height;
    int                 _depth;
    int                 _color_bytes;
    int                 _bytes_per_line;
    int                 _xdpi;
    int                 _ydpi;
    byte*               _data;
    bool                _is_alpha_channel_valid;
};

__ariel_end__

#endif
