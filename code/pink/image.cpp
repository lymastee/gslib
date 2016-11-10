/*
 * Copyright (c) 2016 lymastee, All rights reserved.
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

__pink_begin__

image::image()
{
    _realw = 0;
    _alpha = 0;
    _rawptr = 0;
}

bool image::load(const gchar* n)
{
    return load_image(n, *this);
}

bool image::save(const gchar* n)
{
    return save_image(n, *this);
}

bool image::create(int w, int h, bool a)
{
    destroy();
    if(!w || !h)
        return false;
    _realw = (int)(((uint)(24 * w + 31)) >> 5 << 2);
    int imgsize = _realw * h;
    _rawptr = (unarrtype*)(new byte[imgsize+sizeof(bitmap_header_v1)]); /* allocate! */
    _rawptr->size = sizeof(bitmap_header_v1);
    _rawptr->width = w;
    _rawptr->height = h;
    _rawptr->planes = 1;
    _rawptr->bitcount = 24;
    _rawptr->compression = 0;
    _rawptr->imgsize = imgsize;
    _rawptr->xpels = 0;
    _rawptr->ypels = 0;
    _rawptr->clrused = 0;
    _rawptr->clrimportant = 0;
    if(a) {
        int bitsz = w * h;
        _alpha = new byte[bitsz];
    }
    return true;
}

void image::destroy()
{
    if(_rawptr) {
        delete [] (byte*)_rawptr;   /* deallocate! */
        _rawptr = 0;
    }
    if(_alpha) {
        delete [] _alpha;
        _alpha = 0;
    }
}

void image::clear(const pixel& pix, const rect* rc)
{
    assert(is_valid());
    rect rc1(0, 0, get_width(), get_height());
    if(rc) {
        rc1.left = rc->left;
        rc1.top = rc->top;
        if(rc1.right > rc->right)
            rc1.right = rc->right;
        if(rc1.bottom > rc->bottom)
            rc1.bottom = rc->bottom;
    }
    draw_rect(rc1, pix);
}

void image::set_alpha(int a, const rect* rc)
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

void image::copy(const image* img, bool cpix, bool calpha)
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

void image::copy(const image* img, int x, int y, int cx, int cy, int sx, int sy)
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

void image::draw(const image* img, int x, int y, int cx, int cy, int sx, int sy)
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

void image::draw_text(const gchar* str, int x, int y, const pixel& p)
{
    //fontsys* fsys = wsys_manager::get_singleton_ptr()->get_fontsys();
    fontsys* fsys = 0;
    assert(fsys);
    fsys->draw(*this, str, x, y, p);
}

void image::draw_text(const gchar* str, int x, int y, const font& ft, const pixel& p)
{
    //fontsys* fsys = wsys_manager::get_singleton_ptr()->get_fontsys();
    fontsys* fsys = 0;
    assert(fsys);
    fsys->set_font(ft);
    fsys->draw(*this, str, x, y, p);
}

void image::draw_line(const point& start, const point& end, const pixel& p)
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

void image::draw_rect(const rect& rc1, const pixel& p)
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

void image::set_brigntness(real32 s)
{
    int w = get_width(), h = get_height();
    for(int i = 0; i < h; i ++) {
        pixel* p = get_color(i);
        for(int j = 0; j < w; j ++)
            adjust_brightness(p[j], s);
    }
}

void image::set_brigntness(const image* img, real32 s)
{
    int w = get_width(), h = get_height();
    if(img->get_width() != w || img->get_height() != h)
        return;
    for(int i = 0; i < h; i ++) {
        pixel* p1 = get_color(i);
        const pixel* p2 = img->get_color(i);
        for(int j = 0; j < w; j ++) {
            p1[j] = p2[j];
            adjust_brightness(p1[j], s);
        }
    }
}

void image::set_gray(const image* img)
{
    int w = img->get_width(), h = img->get_height();
    assert(w == get_width() && h == get_height());
    copy(img, false, true);
    for(int i = 0; i < h; i ++) {
        pixel* p = get_color(i);
        const pixel* s = img->get_color(i);
        for(int j = 0; j < w; j ++) {
            int av = s[j].red + s[j].green + s[j].blue;
            av /= 3;
            p[j].red = av;
            p[j].green = av;
            p[j].blue = av;
        }
    }
}

void image::set_inverse(const rect& rc)
{
    int w = rc.right, h = rc.bottom;
    for(int i = rc.top; i < h; i ++) {
        pixel* p = get_color(i);
        for(int j = rc.left; j < w; j ++) {
            p[j].red = 0xff - p[j].red;
            p[j].green = 0xff - p[j].green;
            p[j].blue = 0xff - p[j].blue;
        }
    }
}

void image::fade_to(real32 s)
{
    assert(s >= 0 && s < 1.0);
    int w = get_width(), h = get_height();
    for(int i = 0; i < h; i ++) {
        byte* ptr = get_alpha(i);
        assert(ptr);
        for(int j = 0; j < w; j ++)
            ptr[j] = byte(s * ptr[j]);
    }
}

const byte* image::get_const(const byte* sp, int l, bool flip) const
{
    if(!is_valid() || l >= get_height())
        return 0;
    if(flip) l = flip_line(l);
    assert(l >= 0);
    int r = get_width() * l;
    return sp ? sp + r : 0;
}

const pixel* image::get_const(const pixel* sp, int l, bool flip) const
{
    if(!is_valid() || l >= get_height())
        return 0;
    if(flip) l = flip_line(l);
    assert(l >= 0);
    int r = _realw * l;
    return sp ? (const pixel*)((const byte*)sp + r) : 0;
}

__pink_end__
