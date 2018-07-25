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

#include <ariel/fsyswin32.h>
#include <ariel/image.h>
#include <ariel/scene.h>

__ariel_begin__

static MAT2 _stand_mat = { {0, 1}, {0, 0}, {0, 0}, {0, 1} };

/* used for glyph buffers */
using gs::byte;
static byte* _glyph_buf = 0;
static int _gbsize = 0;

static byte* alloc_glyph_buf(int size = 1024, int predict = 0)
{
    if(!_glyph_buf) {
        _glyph_buf = (byte*)malloc(size);
        if(_glyph_buf)
            _gbsize = size;
        return _glyph_buf;
    }
    if(_gbsize >= size)
        return _glyph_buf;
    if(byte* ptr = (byte*)_expand(_glyph_buf, size)) {
        _gbsize = size;
        return _glyph_buf = ptr;
    }
    free(_glyph_buf);
    _glyph_buf = (byte*)malloc(size += predict);
    _gbsize = _glyph_buf ? size : 0;
    return _glyph_buf;
}

static void free_glyph_buf()
{
    if(_glyph_buf)
        free(_glyph_buf);
    _glyph_buf = 0;
    _gbsize = 0;
}

fsys_win32::fsys_win32()
{
    _container_dc = 0;
    _old_font = 0;
    alloc_glyph_buf();
}

fsys_win32::~fsys_win32()
{
    if(_old_font) {
        assert(_container_dc);
        SelectObject(_container_dc, _old_font);
        _old_font = 0;
    }
    if(_container_dc) {
        DeleteDC(_container_dc);
        _container_dc = 0;
    }
    for(auto i = _font_map.begin(); i != _font_map.end(); ++ i)
        DeleteObject(i->second);
    free_glyph_buf();
}

void fsys_win32::initialize()
{
    HDC hdc = GetDC(0);
    assert(hdc);
    assert(!_container_dc);
    _container_dc = CreateCompatibleDC(hdc);
    ReleaseDC(0, hdc);
}

void fsys_win32::set_font(const font& ft)
{
    assert(_container_dc);
    if(ft.sysfont) {
        HFONT h = (HFONT)SelectObject(_container_dc, (HFONT)ft.sysfont);
        if(!_old_font)
            _old_font = h;
        return;
    }
    auto f = _font_map.find(ft);
    if(f != _font_map.end()) {
        ft.sysfont = (uint)f->second;
        HFONT h = (HFONT)SelectObject(_container_dc, (HFONT)ft.sysfont);
        if(!_old_font)
            _old_font = h;
        return;
    }
    int w = ft.weight % 10;
    w *= 100;
    dword dw = ft.height > 18 ? PROOF_QUALITY : DEFAULT_QUALITY;
    HFONT h = CreateFont(ft.height, ft.width, ft.escape, ft.orient, w,
        (ft.mask & font::ftm_italic) ? 1 : 0, 
        (ft.mask & font::ftm_underline) ? 1 : 0, 
        (ft.mask & font::ftm_strikeout) ? 1 : 0,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, dw, DEFAULT_PITCH, ft.name.c_str()
        );
    ft.sysfont = (uint)h;
    _font_map.insert(std::make_pair(ft, h));
    h = (HFONT)SelectObject(_container_dc, h);
    if(!_old_font)
        _old_font = h;
}

bool fsys_win32::get_size(const gchar* str, int& w, int& h, int len)
{
    if(!str) {
        w = 0, h = 0;
        return false;
    }
    int slen = strtool::length(str);
    if(len < 0 || len > slen)
        len = slen;
    SIZE sz;
    BOOL b = GetTextExtentPoint32(_container_dc, str, len, &sz);
    assert(b);
    w = sz.cx;
    h = sz.cy;
    return true;
}

bool fsys_win32::create_text_image(image& img, const gchar* str, int x, int y, const color& cr, int len)
{
    if(!str || !img.is_valid() || x < 0 || y < 0)
        return false;
    int w = img.get_width(), h = img.get_height();
    if(x > w || y > h)
        return false;
    assert(_glyph_buf && _gbsize);  /* out of memory! */
    GLYPHMETRICS gm;
    TEXTMETRIC tm;
    BOOL b = GetTextMetrics(_container_dc, &tm);
    assert(b);
    /* ensure there is enough memory buffer */
    int r = (int)GetGlyphOutline(_container_dc, 0xBCCD, GGO_GRAY8_BITMAP, &gm, 0, 0, &_stand_mat);
    if(r > _gbsize) {
        alloc_glyph_buf(r + 1024);
        assert(_glyph_buf && _gbsize);
        if(!_glyph_buf || !_gbsize)
            return false;
    }
    if(len < 0)
        len = strtool::length(str);
    /* erase the background, get blank width */
    SIZE sz;
    GetTextExtentPoint32(_container_dc, str, len, &sz);
    rect rcclr(x, y, sz.cx, sz.cy);
    img.clear(color(cr.red, cr.green, cr.blue, 0), &rcclr);
    int alpha = cr.alpha;
    GetTextExtentPoint32(_container_dc, _t(" "), 1, &sz);
    /* end condition */
    const gchar* end = str + len;
    for(;;) {
        uint c = 0;
        str = get_next_char(str, c, end);
        if(!c)  break;
        if(c == _t('\r') || c == _t('\n') || c == _t('\t'))
            continue;
        if(c == tm.tmBreakChar) {
            /* deal with blank! */
            memset(_glyph_buf, 0, _gbsize);
            memset(&gm, 0, sizeof(gm));
            gm.gmBlackBoxX = sz.cx;
            gm.gmCellIncX = (short)sz.cx;
        }
        else {
            /* normal char */
            GetGlyphOutline(_container_dc, c, GGO_GRAY8_BITMAP, &gm, _gbsize, _glyph_buf, &_stand_mat);
        }
        int u = gm.gmBlackBoxX, v = gm.gmBlackBoxY;
        int xbias = gm.gmptGlyphOrigin.x,
            ybias = tm.tmAscent - gm.gmptGlyphOrigin.y;
        if(u & 3) { u &= ~3; u += 4; }
        const byte* ptr = _glyph_buf;
        int vmax = gs_min(v, h-ybias) + y;
        for(int j = y, k = 0; j < vmax; j ++) {
            int grow = x + xbias;
            if(grow >= w)
                break;
            byte* des = img.get_data(grow, j + ybias);
            assert(des);
            int umax = gs_min(u, w-grow);
            for(int i = 0; i < umax; i ++) {
                int cb = (int)ptr[i];
                cb = (cb << 8) - cb;
                cb >>= 6;
                byte* des = img.get_data(grow + i, ybias + j);
                assert(des);
                des[3] = cb * alpha / 255;
            }
            ptr += u;
        }
        x += gm.gmCellIncX;
        y += gm.gmCellIncY;
    }
    return true;
}

bool fsys_win32::create_text_texture(texture2d** tex, const gchar* str, int x, int y, const color& cr, int len)
{
    assert(str);
    scene* scn = scene::get_singleton_ptr();
    assert(scn);
    rendersys* rsys = scn->get_rendersys();
    assert(rsys);
    if(len < 0)
        len = strtool::length(str);
    int w, h;
    get_size(str, w, h, len);
    image img;
    img.create(image::fmt_rgba, w + x, h + y);
    img.enable_alpha_channel(true);
    if(!create_text_image(img, str, x, y, cr, len))
        return false;
    auto* p = rsys->create_texture2d(img, 1, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0);
    assert(p && tex);
    *tex = p;
    return true;
}

void fsys_win32::draw(image& img, const gchar* str, int x, int y, const color& cr, int len)
{
    if(!str || !img.is_valid() || x < 0 || y < 0)
        return;
    int w = img.get_width(), h = img.get_height();
    if(x > w || y > h)
        return;
    assert(_glyph_buf && _gbsize);
    GLYPHMETRICS gm;
    TEXTMETRIC tm;
    BOOL b = GetTextMetrics(_container_dc, &tm);
    assert(b);
    /* ensure there is enough memory buffer */
    int r = (int)GetGlyphOutline(_container_dc, 0xBCCD, GGO_GRAY8_BITMAP, &gm, 0, 0, &_stand_mat);
    if(r > _gbsize) {
        alloc_glyph_buf(r + 1024);
        assert(_glyph_buf && _gbsize);
        if(!_glyph_buf || !_gbsize)
            return;
    }
    /* get blank width */
    SIZE sz;
    GetTextExtentPoint32(_container_dc, _t(" "), 1, &sz);
    /* end condition */
    if(len < 0)
        len = strtool::length(str);
    const gchar* end = str + len;
    for(;;) {
        uint c = 0;
        str = get_next_char(str, c, end);
        if(!c)  break;
        if(c == _t('\r') || c == _t('\n') || c == _t('\t')) continue;
        if(c == tm.tmBreakChar) {
            /* deal with blank! */
            memset(_glyph_buf, 0, _gbsize);
            memset(&gm, 0, sizeof(gm));
            gm.gmBlackBoxX = sz.cx;
            gm.gmCellIncX = (short)sz.cx;
        }
        else {
            /* normal char */
            GetGlyphOutline(_container_dc, c, GGO_GRAY8_BITMAP, &gm, _gbsize, _glyph_buf, &_stand_mat);
        }
        int u = gm.gmBlackBoxX, v = gm.gmBlackBoxY;
        int xbias = gm.gmptGlyphOrigin.x,
            ybias = tm.tmAscent - gm.gmptGlyphOrigin.y;
        if(u & 3) { u &= ~3; u += 4; }
        const byte* ptr = _glyph_buf;
        int vmax = gs_min(y+v, h-ybias);
        for(int j = y, k = 0; j < vmax; j ++) {
            int grow = x + xbias;
            if(grow >= w)
                break;
            int umax = gs_min(u, w-grow);
            color* d = reinterpret_cast<color*>(img.get_data(grow, j + ybias));
            assert(d);
            for(int i = 0; i < umax; i ++, d ++) {
                if(!ptr[i]);
                else if(ptr[i] == 64)
                    *d = cr;
                else {
                    byte a1 = (ptr[i] - 1) << 2;
                    byte a2 = ~a1;
                    d->red = (byte)(((int)d->red * a2 + (int)cr.red * a1) >> 8);
                    d->green = (byte)(((int)d->green * a2 + (int)cr.green * a1) >> 8);
                    d->blue = (byte)(((int)d->blue * a2 + (int)cr.blue * a1) >> 8);
                }
            }
            ptr += u;
        }
        x += gm.gmCellIncX;
        y += gm.gmCellIncY;
    }
}

__ariel_end__
