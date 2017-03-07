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
 
#ifndef image_a41c29de_bd9b_4bbd_9012_6dc956f524b3_h
#define image_a41c29de_bd9b_4bbd_9012_6dc956f524b3_h

#include <gslib/type.h>
#include <gslib/string.h>
#include <pink/dirty.h>

__pink_begin__

struct bitmap_header
{
    uint                size;
    int                 width;
    int                 height;
    word                planes;
    word                bit_count;
    uint                compression;
    uint                image_size;
    int                 x_pels;
    int                 y_pels;
    uint                clr_used;
    uint                clr_important;
};

// todo: remove
#pragma warning(disable: 4200)
template<uint _tsize, uint _tclrt = 0>
struct bitmap_data_v1:
    public bitmap_header
{
    uint    crtable[_tclrt];
    byte    color[_tsize];
};

template<uint _tsize>
struct bitmap_data_v1<_tsize, 0>:
    public bitmap_header
{
    byte    color[_tsize];
};

// change to pitch -1
class image_old//:
    //protected bitmap_header
{
public:
    image_old();
    ~image_old() { destroy(); }
    bool is_valid() const { return _rawptr != 0; }
    //bool load(const gchar* n);
    //bool save(const gchar* n);
    bool create(int w, int h, bool a = false);
    void destroy();
    int get_width() const { return _rawptr ? _rawptr->width : 0; }
    int get_height() const { return _rawptr ? _rawptr->height : 0; }
    int flip_line(int l) const { return get_height() - l - 1; }
    pixel* get_color(int l, bool flip = true) { return get_data((pixel*)(_rawptr->color), l, flip); }
    byte* get_alpha(int l, bool flip = true) { return get_data(_alpha, l, flip); }
    pixel& get_pixel(int x, int y) { return get_color(y)[x]; }
    const pixel* get_color(int l, bool flip = true) const { return get_const((pixel*)(_rawptr->color), l, flip); }
    const byte* get_alpha(int l, bool flip = true) const { return get_const(_alpha, l, flip); }
    const pixel& get_pixel(int x, int y) const { return get_color(y)[x]; }
    void* get_header() const { return (void*)_rawptr; }
    void set_alpha(int a, const rect* rc = 0);
    void clear(const pixel& pix, const rect* rc = 0);
    void copy(const image_old* img, bool cpix, bool calpha);
    void copy(const image_old* img, int x, int y, int cx, int cy, int sx, int sy);
    void draw(const image_old* img, int x, int y) { draw(img, x, y, img->get_width(), img->get_height(), 0, 0); }
    void draw(const image_old* img, int x, int y, int cx, int cy, int sx, int sy);
    void draw_text(const gchar* str, int x, int y, const pixel& p);
    void draw_text(const gchar* str, int x, int y, const font& ft, const pixel& p);
    void draw_line(const point& start, const point& end, const pixel& p);
    void draw_rect(const rect& rc, const pixel& p);

    // move to effects
    //void set_brigntness(real32 s);
    //void set_brigntness(const image* img, real32 s);
    //void set_gray(const image* img);
    //void set_inverse(const rect& rc);
    //void fade_to(real32 s);

private:
    const byte* get_const(const byte* sp, int l, bool flip) const;
    const pixel* get_const(const pixel* sp, int l, bool flip) const;
    template<class _trtty>
    _trtty* get_data(const _trtty* sp, int l, bool flip)
    { return const_cast<_trtty*>(get_const(sp, l, flip)); }

protected:
    int                 _realw;
    byte*               _color;
    byte*               _alpha;

protected:
    /* bitmap data, only header data could be accessed directly */
    typedef bitmap_data_v1<1> unarrtype;
    unarrtype*  _rawptr;
};

class image
{
public:
    enum image_format
    {
        fmt_raw,                    /* raw8 */
        fmt_rgb,                    /* rgb888 */
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
    bool has_alpha() const;
    int get_width() const { return _width; }
    int get_height() const { return _height; }
    int get_bytes_per_line() const { return _bytes_per_line; }
    //byte* get_color(int x, int y) const;
    byte* get_data(int x, int y) const;
    bool load(const string& filepath);
    bool load(const gchar* filepath, int len) { return load(string(filepath, len)); }

protected:
    image_format        _format;
    int                 _width;
    int                 _height;
    int                 _depth;
    int                 _color_bytes;
    int                 _bytes_per_line;
    int                 _xpels_per_meter;
    int                 _ypels_per_meter;
    byte*               _data;
};

__pink_end__

#endif
