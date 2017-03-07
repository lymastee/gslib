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

#include <assert.h>
#include <pink/image.h>
#include <pink/widget.h>
#include <pink/imgloader.h>
#include <pink/imageio.h>

__pink_begin__

image_old::image_old()
{
    _realw = 0;
    _color = 0;
    _alpha = 0;
    //memset(static_cast<bitmap_header*>(this), 0, sizeof(bitmap_header));
}

// bool image::load(const gchar* n)
// {
//     return load_image(n, *this);
// }
// 
// bool image::save(const gchar* n)
// {
//     return save_image(n, *this);
// }

bool image_old::create(int w, int h, bool a)
{
    destroy();
    if(!w || !h)
        return false;
    _realw = (int)(((uint)(24 * w + 31)) >> 5 << 2);
    int imgsize = _realw * h;
    _rawptr = (unarrtype*)(new byte[imgsize+sizeof(bitmap_header)]); /* allocate! */
    _rawptr->size = sizeof(bitmap_header);
    _rawptr->width = w;
    _rawptr->height = h;
    _rawptr->planes = 1;
    _rawptr->bit_count = 24;
    _rawptr->compression = 0;
    _rawptr->image_size = imgsize;
    _rawptr->x_pels = 0;
    _rawptr->y_pels = 0;
    _rawptr->clr_used = 0;
    _rawptr->clr_important = 0;
    if(a) {
        int bitsz = w * h;
        _alpha = new byte[bitsz];
    }
    return true;
}

// bool image::create(int w, int h, bool a)
// {
//     destroy();
//     if(!w || !h)
//         return false;
//     _realw = (int)(((uint)(24 * w + 31)) >> 5 << 2);
//     size = sizeof(bitmap_header);   /* header size */
//     width = w;
//     height = h;
//     planes = 1;
//     bit_count = 24;
//     compression = 0;
//     image_size = _realw * h;
//     x_pels = 0;
//     y_pels = 0;
//     clr_used = 0;
//     clr_important = 0;
//     assert(!_color);
//     _color = new byte[image_size];
//     if(a) {
//         assert(!_alpha);
//         int bitsz = w * h;
//         _alpha = new byte[bitsz];
//     }
//     return true;
// }

void image_old::destroy()
{
    if(_color) {
        delete [] _color;
        _color = 0;
    }
    if(_alpha) {
        delete [] _alpha;
        _alpha = 0;
    }
}

void image_old::clear(const pixel& pix, const rect* rc)
{
}

// void image::clear(const pixel& pix, const rect* rc)
// {
//     assert(is_valid());
//     rect rc1(0, 0, get_width(), get_height());
//     if(rc) {
//         rc1.left = rc->left;
//         rc1.top = rc->top;
//         if(rc1.right > rc->right)
//             rc1.right = rc->right;
//         if(rc1.bottom > rc->bottom)
//             rc1.bottom = rc->bottom;
//     }
//     draw_rect(rc1, pix);
// }

void image_old::set_alpha(int a, const rect* rc)
{
    assert(is_valid() && _alpha);
    if(rc) {
        int size = rc->right - rc->left;
        int imax = flip_line(rc->top);
        for(int i = flip_line(rc->bottom-1); i <= imax; i ++)
            memset(get_alpha(i, false) + rc->left, a, size);
        return;
    }
    int size = _rawptr->width * _rawptr->height;
    memset(_alpha, a, size);
}

void image_old::copy(const image_old* img, bool cpix, bool calpha)
{
    assert(img && is_valid());
    int u = gs_min(get_width(), img->get_width()), 
        v = gs_min(get_height(), img->get_height());
    if(cpix) {
        int s = u * sizeof(pixel);
        for(int i = 0; i < v; i ++)
            memcpy_s(get_color(i), s, img->get_color(i), s);
    }
    if(calpha && _alpha) {
        for(int i = 0; i < v; i ++)
            memcpy_s(get_alpha(i), u, img->get_alpha(i), u);
    }
}

void image_old::copy(const image_old* img, int x, int y, int cx, int cy, int sx, int sy)
{
    if(!is_valid() || !cx || !cy)
        return;
    if(cx < 0 || x + cx > get_width())
        cx = get_width() - x;
    if(cy < 0 || y + cy > get_height())
        cy = get_height() - y;
    if(sx + cx > img->get_width())
        cx = img->get_width() - sx;
    if(sy + cy > img->get_height())
        cy = img->get_height() - sy;
    int s = cx * sizeof(pixel),
        v = flip_line(y),
        i = flip_line(y + cy - 1),
        j = img->flip_line(sy + cy - 1);
    if(img->get_alpha(0)) {
        for( ; i <= v; i ++, j ++) {
            const byte* as = img->get_alpha(j, false) + sx;
            const pixel* src = img->get_color(j, false) + sx;
            byte* ad = get_alpha(i, false) + x;
            pixel* dst = get_color(i, false) + x;
            memcpy_s(ad, cx, as, cx);
            memcpy_s(dst, s, src, s);
        }
    }
    else {
        for( ; i <= v; i ++, j ++)
            memcpy_s(get_color(i, false) + x, s, img->get_color(j, false) + sx, s);
    }
}

void image_old::draw(const image_old* img, int x, int y, int cx, int cy, int sx, int sy)
{
    if(!is_valid() || !cx || !cy)
        return;
    if(cx < 0 || x + cx > get_width())
        cx = get_width() - x;
    if(cy < 0 || y + cy > get_height())
        cy = get_height() - y;
    if(sx + cx > img->get_width())
        cx = img->get_width() - sx;
    if(sy + cy > img->get_height())
        cy = img->get_height() - sy;
    if(!img->get_alpha(0)) {
        int s = cx * sizeof(pixel),
            v = flip_line(y),
            i = flip_line(y + cy - 1),
            j = img->flip_line(sy + cy - 1);
        for( ; i <= v; i ++, j ++)
            memcpy_s(get_color(i, false) + x, s, img->get_color(j, false) + sx, s);
    }
    else {
        int v = flip_line(y),
            i = flip_line(y + cy - 1),
            j = img->flip_line(sy + cy - 1);
        int cvtedx = x * sizeof(pixel),
            cvtedsx = sx * sizeof(pixel);
        for( ; i <= v; i ++, j ++) {
            const byte* a = img->get_alpha(j, false) + sx;
            const byte* s = ((const byte*)img->get_color(j, false)) + cvtedsx;
            byte* d = ((byte*)get_color(i, false)) + cvtedx;
            for(int k = 0; k < cx; k ++) {
                if(!*a) {
                    a ++;
                    s += 3;
                    d += 3;
                }
                else if(*a == 255) {
                    a ++;
                    *d ++ = *s ++;
                    *d ++ = *s ++;
                    *d ++ = *s ++;
                }
                else {
                    byte a1 = *a ++;
                    byte a2 = ~a1;
                    *d ++ = (byte)(((int)*d * a2 + *s ++ * a1) >> 8);
                    *d ++ = (byte)(((int)*d * a2 + *s ++ * a1) >> 8);
                    *d ++ = (byte)(((int)*d * a2 + *s ++ * a1) >> 8);
                }
            }
        }
    }
}

void image_old::draw_text(const gchar* str, int x, int y, const pixel& p)
{
    //fontsys* fsys = wsys_manager::get_singleton_ptr()->get_fontsys();
    fontsys* fsys = 0;
    assert(fsys);
    //fsys->draw(*this, str, x, y, p);
}

void image_old::draw_text(const gchar* str, int x, int y, const font& ft, const pixel& p)
{
    //fontsys* fsys = wsys_manager::get_singleton_ptr()->get_fontsys();
    fontsys* fsys = 0;
    assert(fsys);
    fsys->set_font(ft);
    //fsys->draw(*this, str, x, y, p);
}

void image_old::draw_line(const point& start, const point& end, const pixel& p)
{
    int x = start.x, y = start.y, x2 = end.x, y2 = end.y;
    assert(x >= 0 && y >= 0 && x2 >= 0 && y2 >= 0);
    int w = get_width(), h = get_height();
    if(x >= w)  x = w - 1;
    if(y >= h)  y = h - 1;
    if(x2 > w)  x2 = w;
    if(y2 > h)  y2 = h;
    int dx = x2 - x, dy = y2 - y;
    int ux = ((dx > 0) << 1) - 1;
    int uy = ((dy > 0) << 1) - 1;
    int eps = 0;
    dx = abs(dx), dy = abs(dy);
    if(dx > dy) {
        for( ; x != x2; x += ux) {
            get_color(y)[x] = p;
            eps += dy;
            if((eps << 1) >= dx) {
                y += uy;
                eps -= dx;
            }
        }
    }
    else {
        for( ; y != y2; y += uy) {
            get_color(y)[x] = p;
            eps += dx;
            if((eps << 1) >= dy) {
                x += ux;
                eps -= dy;
            }
        }
    }
}

void image_old::draw_rect(const rect& rc1, const pixel& p)
{
    assert(is_valid());
    rect rc = rc1;
    if(rc.left < 0)
        rc.left = 0;
    if(rc.top < 0)
        rc.top = 0;
    if(rc.right > get_width())
        rc.right = get_width();
    if(rc.bottom > get_height())
        rc.bottom = get_height();
    int size = rc.right - rc.left;
    int imax = flip_line(rc.top);
    for(int i = flip_line(rc.bottom-1); i <= imax; i ++) {
        pixel* ptr = get_color(i, false);
        for(int j = rc.left; j < rc.right; j ++)
            ptr[j] = p;
    }
}

pixel& adjust_brightness(pixel& original, float scale)
{
    assert(scale >= 0 && scale < 2.0f);
    if(scale == 1.0f)
        return original;
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
        return original;
    }
    else {
        float r = (float)original.red * scale;
        original.red = (int)r;
        r = (float)original.green * scale;
        original.green = (int)r;
        r = (float)original.blue * scale;
        original.blue = (int)r;
        return original;
    }
}

// void image::set_brigntness(real32 s)
// {
//     int w = get_width(), h = get_height();
//     for(int i = 0; i < h; i ++) {
//         pixel* p = get_color(i);
//         for(int j = 0; j < w; j ++)
//             adjust_brightness(p[j], s);
//     }
// }

// void image::set_brigntness(const image* img, real32 s)
// {
//     int w = get_width(), h = get_height();
//     if(img->get_width() != w || img->get_height() != h)
//         return;
//     for(int i = 0; i < h; i ++) {
//         pixel* p1 = get_color(i);
//         const pixel* p2 = img->get_color(i);
//         for(int j = 0; j < w; j ++) {
//             p1[j] = p2[j];
//             adjust_brightness(p1[j], s);
//         }
//     }
// }

// void image::set_gray(const image* img)
// {
//     int w = img->get_width(), h = img->get_height();
//     assert(w == get_width() && h == get_height());
//     copy(img, false, true);
//     for(int i = 0; i < h; i ++) {
//         pixel* p = get_color(i);
//         const pixel* s = img->get_color(i);
//         for(int j = 0; j < w; j ++) {
//             int av = s[j].red + s[j].green + s[j].blue;
//             av /= 3;
//             p[j].red = av;
//             p[j].green = av;
//             p[j].blue = av;
//         }
//     }
// }

// void image::set_inverse(const rect& rc)
// {
//     int w = rc.right, h = rc.bottom;
//     for(int i = rc.top; i < h; i ++) {
//         pixel* p = get_color(i);
//         for(int j = rc.left; j < w; j ++) {
//             p[j].red = 0xff - p[j].red;
//             p[j].green = 0xff - p[j].green;
//             p[j].blue = 0xff - p[j].blue;
//         }
//     }
// }

// void image::fade_to(real32 s)
// {
//     assert(s >= 0 && s < 1.0);
//     int w = get_width(), h = get_height();
//     for(int i = 0; i < h; i ++) {
//         byte* ptr = get_alpha(i);
//         assert(ptr);
//         for(int j = 0; j < w; j ++)
//             ptr[j] = byte(s * ptr[j]);
//     }
// }

const byte* image_old::get_const(const byte* sp, int l, bool flip) const
{
    if(!is_valid() || l >= get_height())
        return 0;
    if(flip) l = flip_line(l);
    assert(l >= 0);
    int r = get_width() * l;
    return sp ? sp + r : 0;
}

const pixel* image_old::get_const(const pixel* sp, int l, bool flip) const
{
    if(!is_valid() || l >= get_height())
        return 0;
    if(flip) l = flip_line(l);
    assert(l >= 0);
    int r = _realw * l;
    return sp ? (const pixel*)((const byte*)sp + r) : 0;
}

image::image()
{
    _width = _height = 0;
    _depth = 0;
    _color_bytes = 0;
    _bytes_per_line = 0;
    _data = 0;
    _xpels_per_meter = _ypels_per_meter = 96;
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
    case fmt_raw:
        _depth = 8;
        break;
    case fmt_rgb:
        _depth = 24;
        break;
    case fmt_rgba:
        _depth = 32;
        break;
    default:
        return false;
    }
    _xpels_per_meter = _ypels_per_meter = 96;
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

bool image::has_alpha() const
{
    if(_depth == 32) {
        assert(_format == fmt_rgba);
        return true;
    }
    return false;
}

// byte* image::get_color(int x, int y) const
// {
//     assert(is_valid());
//     if((x >= _width) || (y >= _height)) {
//         assert(!"invalid position.");
//         return 0;
//     }
//     int pos = _bytes_per_line * y + x;
//     return _data + pos;
// }

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

__pink_end__
